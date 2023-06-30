#include "tiles/images.h"

#include "av/sdl2/sdl2.h"
#include "av/sdl2/window.h"
#include "util/compiler.h"
#include "util/int.h"

extern "C" {

// usr/include/objc/NSObjCRuntime.h
typedef unsigned long NSUInteger;

// usr/include/objc/NSObject.h
@protocol NSObject
- (void)release;
@end
@interface NSObject <NSObject>
+ (instancetype)alloc;
- (instancetype)init;
@end

// Foundation/NSObject.h
@protocol NSCopying
@end

// usr/include/objc/objc.h
typedef void* id;

// Foundation/NSObject.h
@protocol NSCoding
@end
@protocol NSSecureCoding <NSCoding>
@end

#pragma clang assume_nonnull begin

// Metal/MTLResource.h
@protocol MTLResource <NSObject>
@end

// Metal/MTLTexture.h
@protocol MTLTexture <MTLResource>
@end

// Metal/MTLRenderPass.h
struct MTLClearColor {
    double red;
    double green;
    double blue;
    double alpha;
};
typedef NSUInteger MTLLoadAction;
#define MTLLoadActionClear 2
typedef NSUInteger MTLStoreAction;
#define MTLStoreActionStore 1
@interface MTLRenderPassAttachmentDescriptor : NSObject <NSCopying>
@property (nullable, nonatomic, strong) id <MTLTexture> texture;
@property (nonatomic) MTLLoadAction loadAction;
@property (nonatomic) MTLStoreAction storeAction;
@end
@interface MTLRenderPassColorAttachmentDescriptor : MTLRenderPassAttachmentDescriptor
@property (nonatomic) MTLClearColor clearColor;
@end
@interface MTLRenderPassColorAttachmentDescriptorArray : NSObject
- (MTLRenderPassColorAttachmentDescriptor *)objectAtIndexedSubscript:(NSUInteger)attachmentIndex;
@end
@interface MTLRenderPassDescriptor : NSObject <NSCopying>
+ (MTLRenderPassDescriptor *)renderPassDescriptor;
@property (readonly) MTLRenderPassColorAttachmentDescriptorArray *colorAttachments;
@end

// Metal/MTLDrawable.h
@protocol MTLDrawable <NSObject>
@end

// Metal/MTLCommandEncoder.h
@protocol MTLCommandEncoder <NSObject>
- (void)endEncoding;
@end

// Metal/MTLRenderCommandEncoder.h
@protocol MTLRenderCommandEncoder <MTLCommandEncoder>
@end

// Metal/MTLCommandBuffer.h
@protocol MTLCommandBuffer <NSObject>
- (void)commit;
- (void)presentDrawable:(id <MTLDrawable>)drawable;
- (nullable id <MTLRenderCommandEncoder>)renderCommandEncoderWithDescriptor:(MTLRenderPassDescriptor*)renderPassDescriptor;
@end

// Metal/MTLCommandQueue.h
@protocol MTLCommandQueue <NSObject>
- (nullable id <MTLCommandBuffer>)commandBuffer;
@end

// Metal/MTLDevice.h
@protocol MTLDevice <NSObject>
- (nullable id <MTLCommandQueue>)newCommandQueue;
@end
id <MTLDevice> __nullable MTLCreateSystemDefaultDevice(void);

// QuartzCore/CAMediaTiming.h
@protocol CAMediaTiming
@end

// QuartzCore/CALayer.h
@interface CALayer : NSObject <NSSecureCoding, CAMediaTiming>
@end

// QuartzCore/CAMetalLayer.h
@protocol CAMetalDrawable <MTLDrawable>
@property(readonly) id<MTLTexture> texture;
@end
@interface CAMetalLayer : CALayer
@property(nullable, retain) id<MTLDevice> device;
- (nullable id <CAMetalDrawable>)nextDrawable;
@end

#pragma clang assume_nonnull end
}

static CAMetalLayer* layer;
static MTLClearColor color = {0., 0., 0., 1.};
static id<MTLCommandQueue> queue;

#define NULL_TEXTURE reinterpret_cast<void*>(1)

void
imageInit() noexcept {
    void* view = SDL_Metal_CreateView(sdl2Window);
    layer = reinterpret_cast<CAMetalLayer*>(SDL_Metal_GetLayer(view));

    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    layer.device = device;

    queue = [device newCommandQueue];
}

Image
imageLoad(StringView path) noexcept {
    Image image = {NULL_TEXTURE, 0, 0, 1, 1};
    return image;
}

void
imageDraw(Image image, float x, float y, float z) noexcept { }

void
imageRelease(Image image) noexcept { }

TiledImage
tilesLoad(StringView path, U32 tileWidth, U32 tileHeight, U32 numAcross,
          U32 numWide) noexcept {
    Image image = {
        NULL_TEXTURE, 0, 0, tileWidth * numAcross, tileHeight * numWide,
    };
    TiledImage tiles = {image, tileWidth, tileHeight, numAcross * numWide};
    return tiles;
}

void
tilesRelease(TiledImage tiles) noexcept { }

Image
tileAt(TiledImage tiles, U32 index) noexcept {
    Image image = {
        NULL_TEXTURE, 0, 0, tiles.tileWidth, tiles.tileHeight,
    };
    return image;
}

void
imagesPrune(Time latestPermissibleUse) noexcept { }

void
imageStartFrame(void) noexcept { }

void
imageEndFrame(void) noexcept {
    @autoreleasepool {
        id<CAMetalDrawable> drawable = [layer nextDrawable];

        color.red = (color.red > 1.0) ? 0 : color.red + 0.01;

        MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor renderPassDescriptor];
        pass.colorAttachments[0].clearColor = color;
        pass.colorAttachments[0].loadAction = MTLLoadActionClear;
        pass.colorAttachments[0].storeAction = MTLStoreActionStore;
        pass.colorAttachments[0].texture = drawable.texture;

        id<MTLCommandBuffer> buffer = [queue commandBuffer];

        id<MTLRenderCommandEncoder> encoder = [buffer renderCommandEncoderWithDescriptor:pass];
        [encoder endEncoding];

        [buffer presentDrawable:drawable];
        [buffer commit];
    }
}

void
imageFlushRects(void) noexcept { }

void
imageFlushImages(void) noexcept { }

void
imageDrawRect(float left, float right, float top, float bottom, float z,
              U32 argb) noexcept { }
