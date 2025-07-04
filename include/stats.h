#ifndef STATS_H
#define STATS_H

#include "public_employee.h"

typedef struct {
    double mean;
    double variance;
    double standard_deviation;
} Stats;

typedef struct {
    const char *label;
    double z;
} GaussianRange;

extern const GaussianRange ranges[];
extern const int range_count;

typedef struct {
    double mu;
    double sigma;
    double x;
} Gaussian;

extern struct timespec t1, t2; // Global time variables for measuring elapsed time

double elapsed_ms(struct timespec start, struct timespec end);
void read_binary_file_and_print(const char* filename);
void calculate_stats(double *data, int size, Stats *stats);
void print_stats(const Stats *stats);
void set_gaussian_parameters(Gaussian *g, const Stats *stats);
double calculate_gaussian(const Gaussian *g);
void plot_gaussian_terminal(double *data, int size);
void data_analisys_by_class(char *field, char *data_searching, char *input_file, char *output_file);

double p_less(const Gaussian *g, double x);
double p_more(const Gaussian *g, double x);
double p_interval(const Gaussian *g, double a, double b);

double normal_cdf(double x, double mu, double sigma);

#endif // STATS_H
