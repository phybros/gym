#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
    int fd = 0;

    // TODO: think about flags etc.
    if (argc == 2) {
        FILE *f = fopen(argv[1], "rb");

        if (f == NULL) {
            perror("Error");
            return 1;
        }

        fd = fileno(f);
    }

    // jsut count the newlines
    int newlines = 0;
    unsigned char buf[1];

    while (read(fd, buf, 1) > 0) {
        if (buf[0] == '\n') {
            newlines++;
        }
    }

    printf("%d", newlines);

    if (fd != 0) {
        printf(" %s", argv[1]);
    }

    printf("\n");
    return 0;
}
