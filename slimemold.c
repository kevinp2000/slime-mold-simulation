#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/wait.h>
#include "slimemold_simulation.h"
#include "encode_video.h"
#include "util.h"

#define FFMPEG_LOG_LEVEL "info"
#define CODEC "libx265"
#define CODEC_PARAM "-x265-params"
#define CODEC_LOG_LEVEL "log-level=error"
#define CRF "20"
#define ENCODER_PRESET "fast"

int parse_int(char *str, char *val, int min) {
    int n = atoi(str);
    if(n < min) {
        fprintf(stderr, "Error: %s < %d\n", val, min);
        exit(1);
    }
    printf("%s=%d\n", val, n);
    return n;
}

float parse_float(char *str, char *val, float min) {
    float n = strtof(str, NULL);
    if(n < min) {
        fprintf(stderr, "Error: %s < %f\n", val, min);
        exit(1);
    }
    printf("%s=%f\n", val, n);
    return n;
}

void write_image(float *map, int width, int height, int fd) {
    ssize_t written;
    // convert float array to byte array
    uint8_t *rounded_map =  malloc_or_die(width * height * sizeof(*rounded_map));
    for (int i = 0; i < width * height; i++) {
        rounded_map[i] = (uint8_t) roundf(map[i]);
    }
    // write the header
    dprintf(fd, "P5\n%d %d 255\n", width, height);
    //write the image
    written = write(fd, rounded_map, width * height * sizeof(uint8_t));
    if (written != width * height) {
        fprintf(stderr, "Error writing to pipe\n");
        exit(1);
    }
    // to debug, write out each frame as an image, need to pass in index i
    /*char name[256];
    sprintf(name, "frame%d.pgm", i);
    FILE *fp = fopen(name, "wb");
    if(!fp) {
        fprintf(stderr, "Could not open file %s\n", name);
        exit(1);
    }
    fprintf(fp, "P5\n%d %d\n255\n", width, height);
    fwrite(rounded_map, sizeof(uint8_t), width * height, fp);
    fclose(fp);*/

    free(rounded_map);
}

void intialize_agents(struct Agent *agents, int nagents, int width, int height) {
    // give each agent a random position and direction
    for (int i = 0; i < nagents; i++) {
        agents[i].x = randf(0, width);
        agents[i].y = randf(0, height);
        agents[i].direction = randf(0, 2 * M_PI);
    }
}

int main(int argc, char *argv[]) {
    // Parse the command line arguments
    if (argc != 13) {
        fprintf(stderr, "usage: %s width height fps seconds nagents movement_speed "
                "trail_deposit_rate movement_noise turn_rate dispersion_rate "
                "evaporation_rate output_file\n", argv[0]);
        exit(1);
    }
    int width = parse_int(argv[1], "width", 1);
    int height = parse_int(argv[2], "height", 1);
    int fps = parse_int(argv[3], "fps", 1);
    int seconds = parse_int(argv[4], "seconds", 1);
    int nagents = parse_int(argv[5], "nagents", 1);
    float movement_speed = parse_float(argv[6], "movement_speed", 0);
    float trail_deposit_rate = parse_float(argv[7], "trail_deposit_rate", 0);
    float movement_noise = parse_float(argv[8], "movement_noise", 0);
    float turn_rate = parse_float(argv[9], "turn_rate", 0);
    float dispersion_rate = parse_float(argv[10], "dispersion_rate", 0);
    float evaporation_rate = parse_float(argv[11], "evaporation_rate", 0);
    char *filename = argv[12];
    printf("\n");

    //initiate FFmpeg
    int outfd;
    pid_t pid;
    if (open_pipe(fps, filename, FAST, &outfd, &pid) == -1) {
        perror("Error");
        exit(1);
    }
    srand(time(0));

    // allocate space for the grid
    struct Map map;
    map.width = width;
    map.height = height;
    map.grid = malloc_or_die(map.width * map.height * sizeof(*(map.grid)));
    // zero out memory
    memset(map.grid, 0, map.width * map.height * sizeof(*(map.grid)));


    // intialize agents
    struct Agent *agents = malloc_or_die(nagents * sizeof(*agents));
    intialize_agents(agents, nagents, width, height);
    for (int i = 0; i < seconds * fps; i++) {
        //printf("----Cycle %d----\n", i);
        // normalize the parameters by fps
        simulate_step(map, agents, nagents,
                        movement_speed/fps, trail_deposit_rate/fps,
                        movement_noise/fps, turn_rate/fps, dispersion_rate/fps,
                        evaporation_rate/fps);
        write_image(map.grid, map.width, map.height, outfd);
    }

    free(map.grid);
    free(agents);
    close_pipe(outfd, pid);
}
