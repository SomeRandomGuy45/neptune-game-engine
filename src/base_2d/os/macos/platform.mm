#import <os/platform.h>
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

namespace neptune {

NSArray *allowedTypes = @[
    [UTType typeWithFilenameExtension:@"project"]
];

BOOL fixActivationPolicy = NO;

char* getFileFromPicker() {
    if (!fixActivationPolicy) {
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        [NSApp finishLaunching];
        [NSApp activateIgnoringOtherApps:YES];
        fixActivationPolicy = YES;
    }
    NSOpenPanel *filePicker = [NSOpenPanel openPanel];
    filePicker.canChooseFiles = YES;
    filePicker.canChooseDirectories = NO;
    filePicker.allowsMultipleSelection = NO;
    filePicker.title = @"Pick a project";
    [filePicker makeKeyAndOrderFront:nil];
    //filePicker.allowedContentTypes = allowedTypes;
    NSInteger result = [filePicker runModal];
    if (result == NSModalResponseOK) {
        NSString *fileUrlStr = [[[filePicker URLs] firstObject] path];
        return strdup([fileUrlStr UTF8String]);
    } else {
        return nullptr;
    }
};

/**
  * Simple testing function to check if it links right...
  * And runs right too
 */
void debugFunction_01() {
    NSLog(@"Hello world!");
}

}