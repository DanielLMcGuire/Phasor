#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LANG 100
#define LINE_LEN 256

typedef struct {
    char name[64];
    int code;
} Language;

int main() {
    char line[LINE_LEN];
    Language langs[MAX_LANG];
    int langCount = 0;
    int totalCode = 0;
    int maxNameLen = 0;

    while (fgets(line, sizeof(line), stdin)) {
        if (strstr(line, "----") || strlen(line) < 5) continue;
        if (strstr(line, "Language") || strstr(line, "files") || strstr(line, "SUM")) continue;

        char langName[64];
        int files, blank, comment, code;

        int matched = sscanf(line, "%63[^0-9] %d %d %d %d", langName, &files, &blank, &comment, &code);
        if (matched == 5) {
            for (int i = strlen(langName) - 1; i >= 0 && (langName[i] == ' ' || langName[i] == '\t'); i--) {
                langName[i] = '\0';
            }
            strcpy(langs[langCount].name, langName);
            langs[langCount].code = code;
            totalCode += code;

            int len = strlen(langName);
            if (len > maxNameLen) maxNameLen = len;

            langCount++;
        }
    }

    printf("Total code lines: %d\n", totalCode);
    printf("Language percentages:\n");
    for (int i = 0; i < langCount; i++) {
        double percent = (totalCode > 0) ? (langs[i].code * 100.0 / totalCode) : 0.0;
        printf("%-*s : %6.2f%% (%d lines)\n", maxNameLen, langs[i].name, percent, langs[i].code);
    }

    return 0;
}
