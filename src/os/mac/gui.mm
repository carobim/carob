#include "os/mac/gui.h"

#include "tiles/world.h"
#include "util/assert.h"
#include "util/compiler.h"
#include "util/string-view.h"
#include "util/string.h"

extern "C" {
I32 chdir(const char *) noexcept;

typedef signed long CFIndex;
typedef signed char BOOL;
typedef unsigned char Boolean;
typedef unsigned char UInt8;

typedef struct CFBundle *CFBundleRef;
typedef const __attribute__((objc_bridge(id))) void* CFTypeRef;
typedef struct __CFURL *CFURLRef;

CFURLRef
CFBundleCopyBundleURL(CFBundleRef bundle) noexcept;
CFBundleRef
CFBundleGetMainBundle() noexcept;
void
CFRelease(CFTypeRef cf) noexcept;
Boolean
CFURLGetFileSystemRepresentation(CFURLRef url,
                                 Boolean resolveAgainstBase,
                                 UInt8 *buffer,
                                 CFIndex maxBufLen) noexcept;
}

typedef long NSInteger;
typedef unsigned long NSUInteger;

typedef NSInteger NSModalResponse;
typedef NSUInteger NSStringEncoding;
#define NSUTF8StringEncoding 4

@protocol NSObject
- (void)release;
@end
@interface NSObject <NSObject>
+ (instancetype)alloc;
- (instancetype)init;
@end

@interface NSString : NSObject
- (nullable instancetype)initWithBytesNoCopy:(const void *)bytes length:(NSUInteger)len encoding:(NSStringEncoding)encoding freeWhenDone:(BOOL)freeBuffer;
@property (nullable, readonly) __strong const char *UTF8String;
@end

@interface NSButton
@end

typedef NSUInteger NSAlertStyle;
enum {
    NSAlertStyleCritical = 2
};

@interface NSAlert : NSObject
@property (copy) NSString *messageText;
@property (copy) NSString *informativeText;
@property NSAlertStyle alertStyle;
- (NSButton *)addButtonWithTitle:(NSString *)title;
- (NSModalResponse)runModal;
@end

void
macSetWorkingDirectory() noexcept {
    UInt8 pathBytes[512];
    CFBundleRef mainBundle;
    CFURLRef url;
    NSString* appPath_;

    mainBundle = CFBundleGetMainBundle();
    assert_(mainBundle);

    url = CFBundleCopyBundleURL(mainBundle);
    assert_(url);

    bool ok = CFURLGetFileSystemRepresentation(url,
                                               true,
                                               pathBytes,
                                               sizeof(pathBytes));
    assert_(ok);

    appPath_ = [[NSString alloc] initWithBytesNoCopy:pathBytes
                                              length:StringView((char*)pathBytes).size + 1
                                            encoding:NSUTF8StringEncoding
                                        freeWhenDone:false];
    assert_(appPath_);

    StringView appPath = [appPath_ UTF8String];
    if (appPath.rfind(StringView(".app", 4)) != appPath.size - 4) {
        // Not in a bundle.
        return;
    }

    // Not sure how to tell Apple LLVM 7.0.0 that Objective-C methods do not
    // throw. Resort to object destruction before we call our next Objective-C
    // method.
    {
        String resourcesPath = appPath;
        resourcesPath << "/Contents/Resources";
        I32 err = chdir(resourcesPath.null());
        assert_(err == 0);
    }

    [appPath_ release];
    CFRelease(url);
}

void
macMessageBox(StringView title, StringView msg) noexcept {
    worldSetPaused(true);

    NSString *nsTitle = [[NSString alloc] initWithBytesNoCopy:title.data
                                                       length:title.size
                                                     encoding:NSUTF8StringEncoding
                                                 freeWhenDone:false];
    NSString *nsMsg = [[NSString alloc] initWithBytesNoCopy:msg.data
                                                     length:msg.size
                                                   encoding:NSUTF8StringEncoding
                                               freeWhenDone:false];

    NSAlert *alert = [[NSAlert alloc] init];
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:nsTitle];
    [alert setInformativeText:nsMsg];
    [alert setAlertStyle:NSAlertStyleCritical];
    [alert runModal];

    [alert release];
    [nsTitle release];
    [nsMsg release];

    worldSetPaused(false);
}
