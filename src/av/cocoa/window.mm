#include "util/int.h"
#include "util/io.h"

// usr/include/objc/objc-api.h
#define OBJC_UNAVAILABLE(_msg) __attribute__((unavailable))
#if __has_feature(objc_arc)
// https://clang.llvm.org/docs/AutomaticReferenceCounting.html
#define OBJC_ARC_UNAVAILABLE OBJC_UNAVAILABLE // Not available when -fobjc-arc
#else
#define OBJC_ARC_UNAVAILABLE
#endif

// usr/include/objc/NSObjCRuntime.h
typedef long NSInteger;
typedef unsigned long NSUInteger;

// usr/include/objc/objc.h
typedef bool BOOL;
typedef void *id;

// usr/include/objc/NSObject.h
@protocol NSObject
- (void)release;
- (instancetype)autorelease OBJC_ARC_UNAVAILABLE;
@end

@interface NSObject <NSObject>
+ (instancetype)alloc;

- (instancetype)init;
@end

#pragma clang arc_cf_code_audited begin
#pragma clang assume_nonnull begin

// CoreGraphics/CGBase.h
#if defined(__LP64__) && __LP64__
# define CGFLOAT_TYPE double
#else
# define CGFLOAT_TYPE float
#endif
typedef CGFLOAT_TYPE CGFloat;

// CoreGraphics/CGGeometry.h
struct CGPoint {
    CGFloat x;
    CGFloat y;
};
struct CGSize {
    CGFloat width;
    CGFloat height;
};
struct CGRect {
    CGPoint origin;
    CGSize size;
};

#pragma clang assume_nonnull end
#pragma clang arc_cf_code_audited end

// Foundation/NSObjCRuntime.h
#define NS_ENUM(...) CF_ENUM(__VA_ARGS__)

// Foundation/NSGeometry.h
typedef CGSize NSSize;
typedef CGRect NSRect;

// Foundation/NSLock.h
@protocol NSLocking
- (void)lock;

- (void)unlock;
@end

@interface NSRecursiveLock : NSObject <NSLocking>
@end

// Foundation/NSObject.h
@protocol NSCoding
@end

@protocol NSSecureCoding <NSCoding>
@end

// OpenGL/gltypes.h
typedef I32 GLint;

// OpenGL/CGLTypes.h
typedef void* CGLContextObj;

#pragma clang assume_nonnull begin

// AppKit/NSResponder.h
@interface NSResponder : NSObject <NSCoding>
@end

// AppKit/NSView.h
@interface NSView : NSResponder <NSAnimatablePropertyContainer, NSUserInterfaceItemIdentification, NSDraggingDestination, NSAppearanceCustomization, NSAccessibilityElement, NSAccessibility>
@end

// AppKit/NSOpenGL.h
typedef U32 NSOpenGLPixelFormatAttribute;
enum {
    NSOpenGLPFADoubleBuffer = 5,
    NSOpenGLPFAColorSize = 8,
    NSOpenGLPFAAlphaSize = 11,
    NSOpenGLPFADepthSize = 12,
    NSOpenGLPFASampleBuffers = 55,
    NSOpenGLPFASamples = 56,
    NSOpenGLPFAMultisample = 59,
    NSOpenGLPFAAccelerated = 73,
    NSOpenGLPFAOpenGLProfile = 99, // OS X 10.7+
};
enum {
    NSOpenGLProfileVersionLegacy = 0x1000, // OS X 10.7+
    NSOpenGLProfileVersion3_2Core = 0x3200, // OS X 10.7+
};
@interface NSOpenGLPixelFormat : NSObject <NSCoding>
- (nullable instancetype)initWithAttributes:(const NSOpenGLPixelFormatAttribute *)attribs;
@end
enum NSOpenGLContextParameter: NSInteger {
    NSOpenGLContextParameterSwapInterval = 222,
};

@interface NSOpenGLContext : NSObject <NSLocking>
- (void)makeCurrentContext;
- (void)setValues:(const GLint *)vals forParameter:(NSOpenGLContextParameter)param;
@property (nullable, readonly) CGLContextObj CGLContextObj;
@end

// AppKit/NSOpenGLView.h
@interface NSOpenGLView : NSView
- (nullable instancetype)initWithFrame:(NSRect)frameRect pixelFormat:(nullable NSOpenGLPixelFormat*)format;
@property (nullable, strong) NSOpenGLContext *openGLContext;
@property (nullable, strong) NSOpenGLPixelFormat *pixelFormat;
@property BOOL wantsBestResolutionOpenGLSurface; // default == false on <=10.14, true on >=10.15; OS X 10.7+
@end

// AppKit/NSWindow.h
@protocol NSWindowDelegate <NSObject>
@end

// CoreVideo/CVBase.h
#define CV_BRIDGED_TYPE(type) __attribute__((objc_bridge(T)))
typedef U64 CVOptionFlags;
struct CVSMPTETime {
    I16 subframes;
    I16 subframeDivisor;
    U32 counter;
    U32 type;
    U32 flags;
    I16 hours;
    I16 minutes;
    I16 seconds;
    I16 frames;
};
struct CVTimeStamp {
    U32 version;
    I32 videoTimeScale;
    I64 videoTime;
    U64 hostTime;
    double rateScalar;
    I64 videoRefreshPeriod;
    CVSMPTETime smpteTime;
    U64 flags;
    U64 reserved;
};

// CoreVideo/CVReturn.h
typedef I32 CVReturn;
enum {
    kCVReturnSuccess = 0,
};

// CoreVideo/CVDisplayLink.h
typedef struct CV_BRIDGED_TYPE(id) __CVDisplayLink *CVDisplayLinkRef;

#pragma clang assume_nonnull end

@class View;

static CVReturn
GlobalDisplayLinkCallback(CVDisplayLinkRef, const CVTimeStamp *, const CVTimeStamp *, CVOptionFlags, CVOptionFlags *,
                          void *);

@interface View : NSOpenGLView <NSWindowDelegate> {
@public
    CVDisplayLinkRef displayLink;
    bool running;
    NSRect windowRect;
    NSRecursiveLock *appLock;
}
@end

@implementation View
// Initialize
- (id)initWithFrame:(NSRect)frame {
    running = true;

    // No multisampling
    U32 samples = 0;

    // Keep multisampling attributes at the start of the attribute lists since code below assumes they are array elements 0 through 4.
    NSOpenGLPixelFormatAttribute windowedAttrs[] =
            {
                    NSOpenGLPFAMultisample,
                    NSOpenGLPFASampleBuffers, samples ? 1U : 0U,
                    NSOpenGLPFASamples, samples,
                    NSOpenGLPFAAccelerated,
                    NSOpenGLPFADoubleBuffer,
                    NSOpenGLPFAColorSize, 32,
                    NSOpenGLPFADepthSize, 24,
                    NSOpenGLPFAAlphaSize, 8,
                    NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy,
                    0
            };

    // Try to choose a supported pixel format
    NSOpenGLPixelFormat *pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:windowedAttrs];

    if (!pf) {
        bool valid = false;
        while (!pf && samples > 0) {
            samples /= 2;
            windowedAttrs[2] = samples ? 1 : 0;
            windowedAttrs[4] = samples;
            pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:windowedAttrs];
            if (pf) {
                valid = true;
                break;
            }
        }

        if (!valid) {
            sout << "OpenGL pixel format not supported.\n";
            return 0;
        }
    }

    self = [super initWithFrame:frame pixelFormat:[pf autorelease]];
    appLock = [[NSRecursiveLock alloc] init];

    return self;
}

- (void)prepareOpenGL {
    [super prepareOpenGL];

    [[self window] setLevel:NSNormalWindowLevel];
    [[self window] makeKeyAndOrderFront:self];

    // Make all the OpenGL calls to setup rendering and build the necessary rendering objects
    [[self openGLContext] makeCurrentContext];
    // Synchronize buffer swaps with vertical refresh rate
    GLint swapInt = 1; // Vsynch on!
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLContextParameterSwapInterval];

    // Create a display link capable of being used with all active displays
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);

    // Set the renderer output callback function
    CVDisplayLinkSetOutputCallback(displayLink, &GlobalDisplayLinkCallback, self);

    CGLContextObj
            cglContext = (CGLContextObj)
    [[self openGLContext] CGLContextObj];
    CGLPixelFormatObj
            cglPixelFormat = (CGLPixelFormatObj)
    [[self pixelFormat] CGLPixelFormatObj];
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);

    GLint dim[2] = {windowRect.size.width, windowRect.size.height};
    CGLSetParameter(cglContext, kCGLCPSurfaceBackingSize, dim);
    CGLEnable(cglContext, kCGLCESurfaceBackingSize);

    [appLock lock];
    CGLLockContext((CGLContextObj)
    [[self openGLContext] CGLContextObj]);
    NSLog(@"Initialize");

    NSLog(@"GL version:   %s", glGetString(GL_VERSION));
    NSLog(@"GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    // Temp
    glClearColor(0.5f, 0.6f, 0.7f, 1.0f);
    glViewport(0, 0, windowRect.size.width, windowRect.size.height);
    glEnable(GL_DEPTH_TEST);
    // End temp
    CGLUnlockContext((CGLContextObj)
    [[self openGLContext] CGLContextObj]);
    [appLock unlock];

    // Activate the display link
    CVDisplayLinkStart(displayLink);
}

// Tell the window to accept input events
- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)mouseMoved:(NSEvent *)event {
    [appLock lock];
    NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
    NSLog(@"Mouse pos: %lf, %lf", point.x, point.y);
    [appLock unlock];
}

- (void)mouseDragged:(NSEvent *)event {
    [appLock lock];
    NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
    NSLog(@"Mouse pos: %lf, %lf", point.x, point.y);
    [appLock unlock];
}

- (void)scrollWheel:(NSEvent *)event {
    [appLock lock];
    NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
    NSLog(@"Mouse wheel at: %lf, %lf. Delta: %lf", point.x, point.y, [event deltaY]);
    [appLock unlock];
}

- (void)mouseDown:(NSEvent *)event {
    [appLock lock];
    NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
    NSLog(@"Left mouse down: %lf, %lf", point.x, point.y);
    [appLock unlock];
}

- (void)mouseUp:(NSEvent *)event {
    [appLock lock];
    NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
    NSLog(@"Left mouse up: %lf, %lf", point.x, point.y);
    [appLock unlock];
}

- (void)rightMouseDown:(NSEvent *)event {
    [appLock lock];
    NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
    NSLog(@"Right mouse down: %lf, %lf", point.x, point.y);
    [appLock unlock];
}

- (void)rightMouseUp:(NSEvent *)event {
    [appLock lock];
    NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
    NSLog(@"Right mouse up: %lf, %lf", point.x, point.y);
    [appLock unlock];
}

- (void)otherMouseDown:(NSEvent *)event {
    [appLock lock];
    NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
    NSLog(@"Middle mouse down: %lf, %lf", point.x, point.y);
    [appLock unlock];
}

- (void)otherMouseUp:(NSEvent *)event {
    [appLock lock];
    NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
    NSLog(@"Middle mouse up: %lf, %lf", point.x, point.y);
    [appLock unlock];
}

- (void)mouseEntered:(NSEvent *)event {
    [appLock lock];
    NSLog(@"Mouse entered");
    [appLock unlock];
}

- (void)mouseExited:(NSEvent *)event {
    [appLock lock];
    NSLog(@"Mouse left");
    [appLock unlock];
}

- (void)keyDown:(NSEvent *)event {
    [appLock lock];
    if ([event isARepeat] == NO) {
        NSLog(@"Key down: %d", [event keyCode]);
    }
    [appLock unlock];
}

- (void)keyUp:(NSEvent *)event {
    [appLock lock];
    NSLog(@"Key up: %d", [event keyCode]);
    [appLock unlock];
}

// Update
- (CVReturn)getFrameForTime:(const CVTimeStamp *)outputTime {
    [appLock lock];

    [[self openGLContext] makeCurrentContext];
    CGLLockContext((CGLContextObj)
    [[self openGLContext] CGLContextObj]);

    NSLog(@"Update");
    // Temp
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // EndTemp

    CGLFlushDrawable((CGLContextObj)
    [[self openGLContext] CGLContextObj]);
    CGLUnlockContext((CGLContextObj)
    [[self openGLContext] CGLContextObj]);

    if (false) { // Update loop returns false
        [NSApp terminate:self];
    }

    [appLock unlock];

    return kCVReturnSuccess;
}

// Resize
- (void)windowDidResize:(NSNotification *)notification {
    NSSize size = [[_window contentView] frame].size;
    [appLock lock];
    [[self openGLContext] makeCurrentContext];
    CGLLockContext((CGLContextObj)
    [[self openGLContext] CGLContextObj]);
    NSLog(@"Window resize: %lf, %lf", size.width, size.height);
    // Temp
    windowRect.size.width = size.width;
    windowRect.size.height = size.height;
    glViewport(0, 0, windowRect.size.width, windowRect.size.height);
    // End temp
    CGLUnlockContext((CGLContextObj)
    [[self openGLContext] CGLContextObj]);
    [appLock unlock];
}

- (void)resumeDisplayRenderer {
    [appLock lock];
    CVDisplayLinkStop(displayLink);
    [appLock unlock];
}

- (void)haltDisplayRenderer {
    [appLock lock];
    CVDisplayLinkStop(displayLink);
    [appLock unlock];
}

// Terminate window when the red X is pressed
- (void)windowWillClose:(NSNotification *)notification {
    if (running) {
        running = false;

        [appLock lock];
        NSLog(@"Cleanup");

        CVDisplayLinkStop(displayLink);
        CVDisplayLinkRelease(displayLink);

        [appLock unlock];
    }

    [NSApp terminate:self];
}

// Cleanup
- (void)dealloc {
    [appLock release];
    [super dealloc];
}
@end

static CVReturn
GlobalDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *now, const CVTimeStamp *outputTime,
                          CVOptionFlags flagsIn, CVOptionFlags *flagsOut, void *displayLinkContext) {
    CVReturn result = [(View *) displayLinkContext getFrameForTime:outputTime];
    return result;
}

int main(int argc, const char *argv[]) {
    // Autorelease Pool:
    // Objects declared in this scope will be automatically
    // released at the end of it, when the pool is "drained".
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    // Create a shared app instance.
    // This will initialize the global variable
    // 'NSApp' with the application instance.
    [NSApplication sharedApplication];

    // Create a window:

    // Style flags
    NSUInteger windowStyle =
            NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask | NSMiniaturizableWindowMask;

    // Window bounds (x, y, width, height)
    NSRect screenRect = [[NSScreen mainScreen] frame];
    NSRect viewRect = NSMakeRect(0, 0, 800, 600);
    NSRect windowRect = NSMakeRect(NSMidX(screenRect) - NSMidX(viewRect),
                                   NSMidY(screenRect) - NSMidY(viewRect),
                                   viewRect.size.width,
                                   viewRect.size.height);

    NSWindow *window = [[NSWindow alloc] initWithContentRect:windowRect
                                                   styleMask:windowStyle
                                                     backing:NSBackingStoreBuffered
                                                       defer:NO];
    [window autorelease];

    // Window controller
    NSWindowController *windowController = [[NSWindowController alloc] initWithWindow:window];
    [windowController autorelease];

    // Since Snow Leopard, programs without application bundles and Info.plist files don't get a menubar
    // and can't be brought to the front unless the presentation option is changed
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    // Next, we need to create the menu bar. You don't need to give the first item in the menubar a name
    // (it will get the application's name automatically)
    id menubar = [[NSMenu new] autorelease];
    id appMenuItem = [[NSMenuItem new] autorelease];
    [menubar addItem:appMenuItem];
    [NSApp setMainMenu:menubar];

    // Then we add the quit item to the menu. Fortunately the action is simple since terminate: is
    // already implemented in NSApplication and the NSApplication is always in the responder chain.
    id appMenu = [[NSMenu new] autorelease];
    id appName = [[NSProcessInfo processInfo] processName];
    id quitTitle = [@"Quit " stringByAppendingString:appName];
    id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle
                                                  action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
    [appMenu addItem:quitMenuItem];
    [appMenuItem setSubmenu:appMenu];

    // Create app delegate to handle system events
    View *view = [[[View alloc] initWithFrame:windowRect] autorelease];
    view->windowRect = windowRect;
    [window setAcceptsMouseMovedEvents:1];
    [window setContentView:view];
    [window setDelegate:view];

    // Set app title
    [window setTitle:appName];

    // Add fullscreen button
    [window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];

    // Show window and run event loop
    [window orderFrontRegardless];
    [NSApp run];

    [pool drain];

    return (0);
}
