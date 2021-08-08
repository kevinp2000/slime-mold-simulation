#ifndef UTIL_H
#define UTIL_H

/**
 * Attempts to malloc size bytes, exits on failure
 */
void* malloc_or_die(size_t size);

/**
 * gets a random double between min and max, not thread safe
 */
double randd(double min, double max);

/**
 * Returns a random number selected from a normal distribution
 */
double normal_dist(double sigma);
#endif
