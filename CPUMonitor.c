#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define DELAY_SECONDS 0.02

// Structure to hold CPU time values
typedef struct {
    unsigned long long user;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle;
    unsigned long long iowait;
    unsigned long long irq;
    unsigned long long softirq;
    unsigned long long steal;
} CPU_Time;

// Function to read CPU time from /proc/stat
void read_cpu_time(CPU_Time *cpu_time) {
    FILE* file = fopen("/proc/stat", "r");
    if (!file) {
        perror("Error opening /proc/stat");
        exit(EXIT_FAILURE);
    }

    fscanf(file, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
           &cpu_time->user, &cpu_time->nice, &cpu_time->system,
           &cpu_time->idle, &cpu_time->iowait, &cpu_time->irq,
           &cpu_time->softirq, &cpu_time->steal);

    fclose(file);
}

// Function to calculate CPU usage percentage
float calculate_cpu_usage(CPU_Time *prev_cpu_time, CPU_Time *curr_cpu_time) {
    unsigned long long prev_idle = prev_cpu_time->idle + prev_cpu_time->iowait;
    unsigned long long idle = curr_cpu_time->idle + curr_cpu_time->iowait;

    unsigned long long prev_non_idle = prev_cpu_time->user + prev_cpu_time->nice +
                                        prev_cpu_time->system + prev_cpu_time->irq +
                                        prev_cpu_time->softirq + prev_cpu_time->steal;
    unsigned long long non_idle = curr_cpu_time->user + curr_cpu_time->nice +
                                    curr_cpu_time->system + curr_cpu_time->irq +
                                    curr_cpu_time->softirq + curr_cpu_time->steal;

    unsigned long long prev_total = prev_idle + prev_non_idle;
    unsigned long long total = idle + non_idle;

    // Calculate difference in total CPU time and idle time
    unsigned long long total_diff = total - prev_total;
    unsigned long long idle_diff = idle - prev_idle;

    // Calculate CPU usage percentage
    float cpu_usage = 100.0 * (1.0 - (idle_diff * 1.0 / total_diff));

    return cpu_usage;
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

    fprintf(output_file, "Time,CPU Usage\n");

    CPU_Time prev_cpu_time, curr_cpu_time;
    read_cpu_time(&prev_cpu_time);

    while (1) {
        usleep(DELAY_SECONDS * 1000000); // Wait for a short interval
        read_cpu_time(&curr_cpu_time);
        float cpu_usage = calculate_cpu_usage(&prev_cpu_time, &curr_cpu_time);
        fprintf(output_file, "%ld,%f\n", time(NULL), cpu_usage);
        fflush(output_file);
        memcpy(&prev_cpu_time, &curr_cpu_time, sizeof(CPU_Time)); // Update previous CPU time
    }

    fclose(output_file);

    return EXIT_SUCCESS;
}
