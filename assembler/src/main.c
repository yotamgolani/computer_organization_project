#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("Assembler starting...\n");
    printf("Input file: %s\n", argv[1]);

    // TODO: Implement assembler logic

    return EXIT_SUCCESS;
}
