#include <stdio.h>
#include <math.h>
#include "stats.h"

const GaussianRange ranges[] = {
    {"30%", 0.39}, {"50%", 0.67}, {"80%", 1.28}, {"90%", 1.64}, {"95%", 1.96}
};
const int range_count = sizeof(ranges) / sizeof(ranges[0]);

void calculate_stats(double* data, int size, Stats* stats) {
    double sum = 0.0, sq = 0.0;
    for (int i = 0; i < size; i++) {
        sum += data[i]; sq += data[i] * data[i];
    }
    stats->mean = sum / size;
    stats->variance = (sq / size) - (stats->mean * stats->mean);
    stats->standard_deviation = sqrt(stats->variance);
}

void print_stats(const Stats* s) {
    printf("Mean: %.2f\nVariance: %.2f\nStandard Deviation: %.2f\n",
        s->mean, s->variance, s->standard_deviation);
}

void set_gaussian_parameters(Gaussian* g, const Stats* s) {
    g->mu = s->mean;
    g->sigma = s->standard_deviation;
}

double calculate_gaussian(const Gaussian* g) {
    double z = (g->x - g->mu) / g->sigma;
    return (1.0 / (g->sigma * sqrt(2 * M_PI))) * exp(-0.5 * z * z);
}

double normal_cdf(double x, double mu, double sigma) {
    return 0.5 * (1.0 + erf((x - mu) / (sigma * sqrt(2.0))));
}

double p_less(const Gaussian* g, double x) {
    return normal_cdf(x, g->mu, g->sigma);
}

double p_more(const Gaussian* g, double x) {
    return 1.0 - normal_cdf(x, g->mu, g->sigma);
}

double p_interval(const Gaussian* g, double a, double b) {
    return normal_cdf(b, g->mu, g->sigma) - normal_cdf(a, g->mu, g->sigma);
}


void plot_gaussian_terminal(double* data, int size) {
    if (size <= 0) {
        printf("No data to plot.\n");
        return;
    }

    Stats s;
    Gaussian g;
    calculate_stats(data, size, &s);
    set_gaussian_parameters(&g, &s);

    printf("\nGaussian Distribution (approx):\nMean (µ): %.2f\nStandard Deviation (σ): %.2f\n", g.mu, g.sigma);
    for (int i = 0; i < range_count; i++) {
        double a = g.mu - ranges[i].z * g.sigma;
        double b = g.mu + ranges[i].z * g.sigma;
        printf("%s range: [%.2f ; %.2f]\n", ranges[i].label, a, b);
    }
    double a = 4000, b = 8000;
    printf("P(<= %.2f) = %.2f%%\n", a, p_less(&g, a) * 100);
    printf("P(>  %.2f) = %.2f%%\n", a, p_more(&g, a) * 100);
    printf("P(%.2f - %.2f) = %.2f%%\n", a, b, p_interval(&g, a, b) * 100);
}
