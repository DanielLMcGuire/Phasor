#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <io.h>
    #include <windows.h>
    #define popen _popen
    #define pclose _pclose
    #define STDIN_FILENO 0
#else
    #include <unistd.h>
    #include <sys/select.h>
#endif

#define MAX_LANG 100
#define LINE_LEN 256
#define MAX_CMD 4096

typedef struct {
    char name[64];
    int code;
} Language;

void print_help(void) {
    printf("clocf - cloc formatter\n\n");
    printf("Usage:\n");
    printf("  cloc [options] <path> | clocf        # Process cloc output\n");
    printf("  clocf [options] <path>               # Run cloc automatically\n");
    printf("  clocf --help                         # Show this help\n\n");
    printf("Examples:\n");
    printf("  cloc . | clocf                       # Pipe cloc output\n");
    printf("  clocf src/                           # Auto-run cloc on src/\n");
    printf("  clocf --exclude-dir=vendor .         # Pass options to cloc\n\n");
}

int stdin_has_data(void) {
#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    if (!GetConsoleMode(hStdin, &mode)) {
        /* Not a console, likely a pipe */
        return 1;
    }
    DWORD events;
    if (GetNumberOfConsoleInputEvents(hStdin, &events) && events > 1) {
        return 1;
    }
    return 0;
#else
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
#endif
}

int main(int argc, char *argv[]) {
    if (argc > 1 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        print_help();
        return 0;
    }

    FILE *input = stdin;
    
    if (argc > 1) {
        char cmd[MAX_CMD];
        size_t pos = 0;
        const char *cloc_cmd = "cloc";
        size_t cloc_len = strlen(cloc_cmd);
        
        if (cloc_len >= MAX_CMD) {
            fprintf(stderr, "Error: Command too long\n");
            return 1;
        }
        
        memcpy(cmd, cloc_cmd, cloc_len);
        pos = cloc_len;
        
        for (int i = 1; i < argc; i++) {
            size_t arg_len = strlen(argv[i]);
            int needs_quotes = (strchr(argv[i], ' ') != NULL) ? 1 : 0;
            size_t required = pos + 1 + (needs_quotes ? 2 : 0) + arg_len;
            
            if (required >= MAX_CMD) {
                fprintf(stderr, "Error: Command too long\n");
                return 1;
            }
            
            cmd[pos++] = ' ';
            if (needs_quotes) {
                cmd[pos++] = '"';
            }
            memcpy(cmd + pos, argv[i], arg_len);
            pos += arg_len;
            if (needs_quotes) {
                cmd[pos++] = '"';
            }
        }
        cmd[pos] = '\0';
        
        input = popen(cmd, "r");
        if (!input) {
            fprintf(stderr, "Error: Failed to run cloc command\n");
            return 1;
        }
    } 
    else if (!stdin_has_data()) {
        fprintf(stderr, "Error: No input provided\n\n");
        print_help();
        return 1;
    }

    char line[LINE_LEN];
    Language langs[MAX_LANG];
    int langCount = 0;
    int totalCode = 0;
    int maxNameLen = 0;

    while (fgets(line, sizeof(line), input)) {
        size_t line_len = strlen(line);
        if (strstr(line, "----") || line_len < 5) continue;
        if (strstr(line, "Language") || strstr(line, "files") || strstr(line, "SUM")) continue;

        char langName[64];
        int files, blank, comment, code;

        int matched = sscanf(line, "%63[^0-9] %d %d %d %d", langName, &files, &blank, &comment, &code);
        if (matched == 5) {
            size_t name_len = strlen(langName);
            for (size_t i = name_len; i > 0 && (langName[i - 1] == ' ' || langName[i - 1] == '\t'); i--) {
                langName[i - 1] = '\0';
                name_len = i - 1;
            }
            
            if (langCount < MAX_LANG) {
                size_t copy_len = name_len < 63 ? name_len : 63;
                memcpy(langs[langCount].name, langName, copy_len);
                langs[langCount].name[copy_len] = '\0';
                langs[langCount].code = code;
                totalCode += code;

                if ((int)name_len > maxNameLen) {
                    maxNameLen = (int)name_len;
                }

                langCount++;
            }
        }
    }

    if (argc > 1) {
        pclose(input);
    }

    printf("Total code lines: %d\n", totalCode);
    printf("Language percentages:\n");
    for (int i = 0; i < langCount; i++) {
        double percent = (totalCode > 0) ? (langs[i].code * 100.0 / totalCode) : 0.0;
        printf("%-*s : %6.2f%% (%d lines)\n", maxNameLen, langs[i].name, percent, langs[i].code);
    }

    return 0;
}