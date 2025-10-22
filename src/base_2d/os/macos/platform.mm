#import <os/platform.h>
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

char* getFileFromPicker() {
    NSArray* allowedTypes = @[@"project"];
    NSOpenPanel *filePicker = [NSOpenPanel openPanel];
    filePicker.canChooseFiles = YES;
    filePicker.canChooseDirectories = NO;
    filePicker.allowsMultipleSelection = NO;
    filePicker.message = @"Pick a project";
    filePicker.allowedContentTypes = allowedTypes;
    __block NSString *fileUrlStr;
    [filePicker beginWithCompletionHandler:^(NSModalResponse result) {
        if (!NSModalResponseOK) {
            NSLog(@"Error: Unable to finish request!");
            return ;
        }
        fileUrlStr = [[[filePicker URLs] firstObject] absoluteString];
    }];
    return strdup([fileUrlStr UTF8String]);
};