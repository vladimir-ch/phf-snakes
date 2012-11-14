#ifndef __DATA_H_INCLUDED__
#define __DATA_H_INCLUDED__ 

#include "array.h"

#include <string>

class Phf_snakes_data {
public:
    void read_from_file(std::string const& filename);
    void print () const;

    double h;
    double tau;
    double a;
    double F;
    double lambda;
    double sigma;
    double gs_conv_tolerance;
    bool add_noise;
    bool save_images, save_gnuplot;
    int save_every_n_step;
    int check_every_n_step;
    double C_s;
    int max_gs_iterations;
    std::string ini_filename;
    std::string P0_filename;
    std::string output_path;

    int size_x, size_y;

    array_t p_old, p;
    array_t gx, gy, gh, gradp, gradpx, gradpy;

    std::vector<double> conv_diff, stat_diff;
    bool solve_end, step_end;

private:
    void compute_gh(array_t const& P0_smooth);
    double g (double s) const {
        return 1.0/(1.0 + lambda*s*s);
    }

    std::string problem_name_;
};

#endif /* __DATA_H_INCLUDED__ */
