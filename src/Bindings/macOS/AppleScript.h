#ifndef APPLESCRIPT_RUNNER_H
#define APPLESCRIPT_RUNNER_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int   success;
    char* output;
    char* error;
    int   errorCode;
} AppleScriptResult;

AppleScriptResult executeAppleScript(const char* script);
void              freeAppleScriptResult(AppleScriptResult* result);

#ifdef __cplusplus
}
#endif

#endif /* APPLESCRIPT_RUNNER_H */