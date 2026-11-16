#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#define COLS 16

char * COLOR_RED = "\e[1;31m";
char * COLOR_GREEN = "\e[1;32m";
char * COLOR_YELLOW = "\e[1;33m";
char * COLOR_BLUE = "\e[1;34m";
char * COLOR_RESET = "\e[0m";

int main(int argc, char **argv) {
    // requires a filename as the first arg
    // and then output the file's bytes like this
    // 00000000: 0000 0000 0000 0000 abcdefghijklmnop
    // 00000001: 0000 0000 0000 0000 abcdefghijklmnop

    int colors = 1;
    if (isatty(STDOUT_FILENO)) {
        if (getenv("NO_COLOR")) {
            colors = 0;
        }
    } else {
        colors = 0;
    }

    if (!colors) {
        COLOR_RED = "";
        COLOR_GREEN = "";
        COLOR_YELLOW = "";
        COLOR_RESET = "";
    }

    // TODO: do something better with the args rather than just hardcoding argv[1]

    // need filename!
    if (argc != 2) {
        printf("Error: filename is required\n");
        return 1;
    }

    FILE *f = fopen(argv[1], "rb");

    if (f == NULL) {
        // error opening the file for some reason
        perror("Error");
        fclose(f);
        return 1;
    }

    int line[COLS];
    int pos = 0;
    int currentch;
    int processing = 1;

    while (processing) {
        printf("%08x: ", pos);

        // first pass
        for (int i = 0; i < COLS; i++) {
            currentch = fgetc(f);

            line[i] = currentch;

            if (currentch == EOF) {
                processing = 0;
                break;
            }
        }

        // second pass
        for (int i = 0; i < COLS; i++) {
            if (line[i] == EOF) break;

            if (line[i] == 0) {
                printf("%s00", COLOR_RESET);
            } else if (line[i] == 0xff) {
                printf("%sff", COLOR_BLUE);
            } else if (line[i] == '\n' || line[i] == '\t' || line[i] == '\v' || line[i] == '\f' || line[i] == '\r') {
                printf("%s%02x", COLOR_YELLOW, line[i]);
            } else if (line[i] >= 32 && line[i] <= 126) {
                printf("%s%02x", COLOR_GREEN, line[i]);
            } else if (line[i] < 32 || line[i] >= 127) {
                printf("%s%02x", COLOR_RED, line[i]);
            }

            if (i % 2 != 0)
                printf(" ");
        }

        printf(" ");

        // third pass!?!?
        for (int i = 0; i < COLS; i++) {
            if (line[i] == EOF) break;

            if (line[i] == 0) {
                printf("%s.", COLOR_RESET);
            } else if (line[i] == 0xff) {
                printf("%s.", COLOR_BLUE);
            } else if (line[i] == '\n' || line[i] == '\t' || line[i] == '\v' || line[i] == '\f' || line[i] == '\r') {
                printf("%s.", COLOR_YELLOW);
            } else if (line[i] >= 32 && line[i] <= 126) {
                printf("%s%c", COLOR_GREEN, line[i]);
            } else if (line[i] < 32 || line[i] >= 127) {
                printf("%s.", COLOR_RED);
            }
        }

        printf("%s\n", COLOR_RESET);
        pos += COLS;
    }

    fclose(f);
    return 0;
}
