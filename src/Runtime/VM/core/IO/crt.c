#include "IO.h"
#include <stdio.h>
#include <stdlib.h>

void asm_print_stdout(const char* s, int64_t len) {
    fwrite(s, 1, (size_t)len, stdout);
    fflush(stdout);
}

void asm_print_stderr(const char* s, int64_t len) {
    fwrite(s, 1, (size_t)len, stderr);
    fflush(stderr);
}

int64_t asm_system(const char* cmd) {
    return (int64_t)system(cmd);
}
