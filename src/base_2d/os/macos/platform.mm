#import <os/platform.h>
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

namespace neptune {

NSArray *allowedTypes = @[
    [UTType typeWithFilenameExtension:@"project"]
];

BOOL fixActivationPolicy = NO;

std::string getFileFromPicker(const filePickerSettings& settings) {
    if (!fixActivationPolicy) {
        fixFileExplorerPolicy();
    }
    NSOpenPanel *filePicker = [NSOpenPanel openPanel];
    filePicker.canChooseFiles = YES;
    filePicker.canChooseDirectories = NO;
    filePicker.allowsMultipleSelection = NO;
    filePicker.title = @"Pick a project";
    [filePicker makeKeyAndOrderFront:nil];
    NSInteger result = [filePicker runModal];
    if (result == NSModalResponseOK) {
        NSURL *url = [[filePicker URLs] firstObject];
        NSString *filePath = [url path];

        if (!filePath) {
            return std::string();
        }

        return std::string([filePath UTF8String] ?: "");
    } else {
        return std::string();
    }
};

void fixFileExplorerPolicy() {
    if (!fixActivationPolicy) {
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        [NSApp finishLaunching];
        [NSApp activateIgnoringOtherApps:YES];
        fixActivationPolicy = YES;
    }
}

/**
  * Simple testing function to check if it links right...
  * And runs right too
 */
void debugFunction_01() {
    NSLog(@"Hello world!");
}

int popUpWindow(const char* title, const char* message) {
    // todo
    NSLog(@"%s: %s", title, message);
    return 0;
}

}