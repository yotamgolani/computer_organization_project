#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <program_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("Simulator starting...\n");
    printf("Program file: %s\n", argv[1]);

    // TODO: Implement simulator logic

    return EXIT_SUCCESS;
}
