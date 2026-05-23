#import <os/platform.h>
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

namespace neptune {

NSArray *allowedTypes = @[
    [UTType typeWithFilenameExtension:@"project"]
];

BOOL fixActivationPolicy = NO;

std::string getFileFromPicker() {
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
    //NSLog(@"%s: %s", title, message);
    NSString *titleStr = [NSString stringWithUTF8String:title];
    NSString *messageStr = [NSString stringWithUTF8String:message];
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:messageStr];
    [alert setInformativeText:titleStr];
    [alert setAlertStyle:NSAlertStyleWarning];
    [alert addButtonWithTitle:@"OK"];
    [alert addButtonWithTitle:@"Cancel"];
    NSInteger result = [alert runModal];
    if (result == NSAlertFirstButtonReturn) {
        NSLog(@"OK clicked!");
        return 1;
    } else if (result == NSAlertSecondButtonReturn) {
        return 2;
    }
    return 0;
}

}