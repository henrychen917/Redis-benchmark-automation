#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define DELAY_SECONDS 0.01

// Function to read total memory and free memory from /proc/meminfo
void read_system_memory_usage(unsigned long long *total_memory, unsigned long long *free_memory) {
    FILE* file = fopen("/proc/meminfo", "r");
    if (!file) {
        perror("Error opening /proc/meminfo");
        exit(EXIT_FAILURE);
    }

    char buffer[256];
    // Read total memory
    fgets(buffer, sizeof(buffer), file);
    sscanf(buffer, "MemTotal: %llu kB", total_memory);

    // Read free memory
    fgets(buffer, sizeof(buffer), file);
    sscanf(buffer, "MemFree: %llu kB", free_memory);

    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <output_file.csv>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *output_file = fopen(argv[1], "w");
    if (!output_file) {
        perror("Error opening output file");
        return EXIT_FAILURE;
    }

    fprintf(output_file, "Time,Total Memory (bytes),Free Memory (bytes)\n");

    while (1) {
        unsigned long long total_memory, free_memory;
        read_system_memory_usage(&total_memory, &free_memory);
        fprintf(output_file, "%ld,%llu,%llu\n", time(NULL), total_memory * 1024, free_memory * 1024);
        fflush(output_file);
        usleep(DELAY_SECONDS * 1000000); // usleep takes microseconds
    }

    fclose(output_file);

    return EXIT_SUCCESS;
}
