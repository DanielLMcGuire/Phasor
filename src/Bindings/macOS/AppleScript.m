#import <Foundation/Foundation.h>

typedef struct {
    int    success;
    char*  output;
    char*  error;
    int    errorCode;
} AppleScriptResult;

static char* copyString(NSString* str) {
    if (!str) return NULL;
    const char* utf8 = [str UTF8String];
    if (!utf8) return NULL;
    size_t len = strlen(utf8) + 1;
    char* buf = (char*)malloc(len);
    if (buf) memcpy(buf, utf8, len);
    return buf;
}

AppleScriptResult executeAppleScript(const char* script) {
    AppleScriptResult result = {0, NULL, NULL, 0};

    if (!script) {
        result.error = copyString(@"NULL script pointer");
        return result;
    }

    @autoreleasepool {
        NSString* nsScript = [NSString stringWithUTF8String:script];
        if (!nsScript) {
            result.error = copyString(@"Invalid UTF-8 in script string");
            return result;
        }

        NSAppleScript*          as         = [[NSAppleScript alloc] initWithSource:nsScript];
        NSDictionary*           errorInfo  = nil;
        NSAppleEventDescriptor* descriptor = [as executeAndReturnError:&errorInfo];

        if (descriptor) {
            result.success = 1;
            result.output  = copyString([descriptor stringValue]);
        } else {
            result.success = 0;
            if (errorInfo) {
                result.error     = copyString(errorInfo[NSAppleScriptErrorMessage]);
                result.errorCode = [errorInfo[NSAppleScriptErrorNumber] intValue];
            }
        }
    }

    return result;
}

void freeAppleScriptResult(AppleScriptResult* r) {
    if (!r) return;
    free(r->output); r->output = NULL;
    free(r->error);  r->error  = NULL;
}