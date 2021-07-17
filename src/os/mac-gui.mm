#include "os/mac-gui.h"

#include "core/world.h"
#include "util/assert.h"
#include "util/compiler.h"
#include "util/string-view.h"

extern "C" {
int chdir(const char *) noexcept;

typedef signed long CFIndex;
typedef signed char BOOL;
typedef unsigned char Boolean;
typedef unsigned char UInt8;

typedef struct CFBundle *CFBundleRef;
typedef const __attribute__((objc_bridge(id))) void* CFTypeRef;
typedef struct __CFURL *CFURLRef;

CFURLRes
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

typedef long NSInteger;
typedef unsigned long NSUInteger;

typedef NSInteger NSModalResponse;
typedef NSUInteger NSStringEncoding;
#define NSUTF8StringEncoding 4

@protocol NSObject
- (oneway void)release;
@end
@interface NSObject <NSObject>
+ (instancetype)alloc;
- (instancetype)init;
@end

@interface NSString : NSObject
- (nullable instancetype)initWithBytesNoCopy:(const void *)bytes length:(NSUInteger)len encoding:(NSStringEncoding)encoding freeWhenDone:(BOOL)freeBuffer;
@end

@interface NSFileManager : NSObject
+ (NSFileManager *)defaultManager;
// Compile error on macOS 10.11 El Capitan.
//@property (class, readonly, strong) NSFileManager *defaultManager;
- (BOOL)changeCurrentDirectoryPath:(NSString *)path;
@end
}

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
    NSString* appPath;

    mainBundle = CFBundleGetMainBundle();
    assert_(mainBundle);

    url = CFBundleCopyBundleURL(mainBundle);
    assert_(url);

    bool ok = CFURLGetFileSystemRepresentation(url,
                                             true,
                                             pathBytes,
                                             sizeof(pathBytes));
    (void)ok;
    assert_(ok);

    appPath = [[NSString alloc] initWithBytesNoCopy:pathBytes
                                             length:StringView((char*)pathBytes).size + 1
                                           encoding:NSUTF8StringEncoding
                                       freeWhenDone:false];
    assert_(appPath);

    BOOL ok2 = [[NSFileManager defaultManager] changeCurrentDirectoryPath:appPath];
    (void)ok2;
    assert_(ok2);

    int err = chdir("Contents/Resources");
    (void)err;
    // FIXME: This assert will fail if we are not in a bundle. Detect when
    //        we are not in a bundle before this point and bail out earlier.
    //assert_(err == 0);

    [appPath release];
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
