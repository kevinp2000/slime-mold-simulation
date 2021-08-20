#ifndef SLIMEMOLD_SIMULATION_H
#define SLIMEMOLD_SIMULATION_H

struct Agent {
    double direction;
    double x;
    double y;
};

struct Map {
    double *grid;
    int width;
    int height;
};

// Parameters that control the simulation
struct Behavior {
    double movement_speed;
    double trail_deposit_rate;
    double movement_noise;
    double turn_rate;
    double sensor_length;
    double sensor_angle_factor;
    double dispersion_rate;
    double evaporation_rate_exp;
    double evaporation_rate_lin;
    double trail_max;
};

// agent_pos_frequency doesn't need to be initialized. This is just to avoid repeatedly calling malloc and free
void simulate_step(struct Map *p_map, struct Agent *agents, int nagents, int *agent_pos_frequency, struct Behavior behavior, unsigned int *seeds);
#endif
