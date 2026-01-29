#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int max_lines = -1;
    int verbose = 0;
    int opt;

    long lines = 0;
    long characters = 0;
    char buffer[1024];

    while ((opt = getopt(argc, argv, "n:v")) != -1) {
        switch (opt) {
            case 'n':
                max_lines = atoi(optarg);
                break;
            case 'v':
                verbose = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-n max] [-v]\n", argv[0]);
                return 1;
        }
    }

    while (fgets(buffer, sizeof(buffer), stdin)) {
        lines++;
        characters += strlen(buffer);

        if (verbose) {
            fprintf(stderr, "%s", buffer);
        }

        if (max_lines > 0 && lines >= max_lines) {
            break;
        }
    }

    fprintf(stderr, "\nLines: %ld\nCharacters: %ld\n", lines, characters);
    return 0;
}
