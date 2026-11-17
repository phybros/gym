#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
    int fd = 0;

    if (argc == 2) {
        FILE *f = fopen(argv[1], "rb");

        if (f == NULL) {
            perror("Error");
            return 1;
        }

        fd = fileno(f);
    }


    int newlines = 0;
    int ch;
    int buf[1];

    while (read(fd, buf, 1) != 0) {
        if (buf[0] == '\n') {
            newlines++;
        }
    }

    printf("Lines: %d\n", newlines);
    return 0;
}
