#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

#define DEFAULT_BUFFER 4096

int main(int argc, char *argv[]) {
    FILE *input = stdin;
    char *filename = NULL;
    int buffer_size = DEFAULT_BUFFER;
    int opt;

    // Parse command-line arguments
    while ((opt = getopt(argc, argv, "f:b:")) != -1) {
        switch (opt) {
            case 'f':
                filename = optarg;
                break;
            case 'b':
                buffer_size = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-f file] [-b size]\n", argv[0]);
                return 1;
        }
    }

    if (filename != NULL) {
        input = fopen(filename, "r");
        if (input == NULL) {
            perror("Failed to open input file");
            return 1;
        }
    }

    char *buffer = malloc(buffer_size);
    if (!buffer) {
        perror("Memory allocation failed");
        return 1;
    }

    size_t bytes;
    while ((bytes = fread(buffer, 1, buffer_size, input)) > 0) {
        fwrite(buffer, 1, bytes, stdout);
    }

    free(buffer);
    if (input != stdin) fclose(input);

    return 0;
}
