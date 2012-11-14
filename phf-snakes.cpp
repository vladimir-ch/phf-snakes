//
//  Copyright (c) 2008-2012 Vladimir Chalupecky
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to
//  deal in the Software without restriction, including without limitation the
//  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
//  sell copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
//  IN THE SOFTWARE.

///////////////////////////////////////////////////////////////////////////////
// Implementation of the image segmentation algorithm based on phase-field
// approach to active contours as described in the paper M. Benes, V.
// Chalupecky, and K. Mikula, Geometrical image segmentation by the Allen-Cahn
// equation, Applied Numerical Mathematics, 51 (2004), pp. 187—205
///////////////////////////////////////////////////////////////////////////////

#ifdef _OPENMP
#include <omp.h>
#endif

#include "array.h"
#include "data.h"
#include "exceptions.h"
#include "image_io.h"
#include "phf-snakes.h"
#include "utils.h"

#include <iostream>
#include <cstdlib>

int main(int ac, char* av[])
{
    std::cout << "phf-snakes 1.0 http://github.com/vladimir-ch/phf-snakes/\nCopyright (c) 2008-2012 Vladimir Chalupecky\n";

    Phf_snakes_data shared_data;

    try {
        shared_data.read_from_file("phf-snakes.dat");
    }
    catch (boost::exception & e) {
        std::cerr << boost::diagnostic_information(e);
        return EXIT_FAILURE;
    }

#ifdef _OPENMP
#pragma omp parallel default(shared)
    {
        int tid = omp_get_thread_num();
        int nthreads = omp_get_num_threads();
#pragma omp single
        {
            std::cout << "With OpenMP, number of threads = " << nthreads << std::endl;
            shared_data.print();
        }
#else
    {
        int tid = 0;
        int nthreads = 1;
        std::cout << "Without OpenMP\n";
#endif
        Phf_snakes problem(shared_data, tid, nthreads);
        problem.solve();
    }
    std::cout << std::endl;

    return EXIT_SUCCESS;
}

Phf_snakes::Phf_snakes(Phf_snakes_data& shared_data, int tid, int nthreads)
    : shared_data_(shared_data)
    , tid_(tid)
    , nthreads_(nthreads)
{
    size_x = shared_data_.size_x;
    size_y = shared_data_.size_y;
    int block_size = size_y / nthreads_;
    int leftover   = size_y % nthreads_;
    y_start = tid_ * block_size + std::min(tid_, leftover);
    y_end = y_start + block_size;
    if (leftover > tid_) {
        y_end = y_end + 1;
    }
    h = shared_data_.h;
    h_pow2_inv = 1.0/(h*h);
    xi = h;
    a = shared_data_.a;
    tau = xi*xi/a;
    F = shared_data_.F;
    stationarity_test_constant = shared_data_.C_s*tau;
#pragma omp single
    {
        shared_data_.conv_diff.resize(nthreads_, 0.0);
        shared_data_.stat_diff.resize(nthreads_, 0.0);
        shared_data_.step_end = false;
        shared_data_.solve_end = false;
    }
}

void Phf_snakes::solve() {
    int nstep = 0;
    do {
        step();
        ++nstep;

        if (nstep % shared_data_.check_every_n_step == 0) {
            shared_data_.stat_diff[tid_] = compute_difference();
#pragma omp barrier
#pragma omp single
            {
                double global_stat_diff = shared_data_.stat_diff[0];
                for (int i = 1; i < nthreads_; ++i) {
                    global_stat_diff += shared_data_.stat_diff[i];
                }
                shared_data_.solve_end = global_stat_diff < stationarity_test_constant;
                std::cout << "Time step: " << std::setw(5) << nstep
                    // << ", Gauss-Seidel iterations: " << std::setw(6) << gs_iterations
                          << ", diff = " << std::setw(12) << std::setprecision(5) << global_stat_diff
                          << ", stop diff = " << std::setw(12) << std::setprecision(5) << stationarity_test_constant
                          << "\r";
                std::cout.flush();
            }
#pragma omp barrier
        }

        if (nstep % shared_data_.save_every_n_step == 0) {
#pragma omp barrier
#pragma omp single
            {
                if (shared_data_.save_images) {
                    write_png(shared_data_.output_path + "p-" + to_string(nstep/shared_data_.save_every_n_step, 6) + ".png", shared_data_.p);
                }
                if (shared_data_.save_gnuplot) {
                    write_gnuplot(shared_data_.output_path + "p-" + to_string(nstep/shared_data_.save_every_n_step, 6) + ".dat", shared_data_.p);
                }
            }
#pragma omp barrier
        }
    } while (not shared_data_.solve_end);

#pragma omp barrier
#pragma omp single
    {
        if (nstep % shared_data_.save_every_n_step != 0) {
            if (shared_data_.save_images) {
                write_png(shared_data_.output_path + "p-" + to_string(nstep/shared_data_.save_every_n_step + 1, 6) + ".png", shared_data_.p);
            }
            if (shared_data_.save_gnuplot) {
                write_gnuplot(shared_data_.output_path + "p-" + to_string(nstep/shared_data_.save_every_n_step + 1, 6) + ".dat", shared_data_.p);
            }
        }
    }
}

double Phf_snakes::f0 (double s) const {
    return -a*s*(s - 1)*(s - 0.5);
}

double Phf_snakes::compute_difference() {
    double M = h_pow2_inv/((size_x - 1)*(size_y - 1)); // size of the domain
    double errSum = 0.0;
    for (int y = y_start; y < y_end; ++y) {
        for (int x = 0; x < size_x; ++x) {
            errSum += fabs(shared_data_.p[x][y] - shared_data_.p_old[x][y]);
        }
    }
    return M*errSum;
}

void Phf_snakes::step() {
    double pp, up, dp, lp, rp;
    double ug, dg, lg, rg, gg, gp;
    double local_diff;

    for (int y = y_start; y < y_end; y++) {
        for (int x = 0; x < size_x; x++) {
            shared_data_.p_old[x][y] = shared_data_.p[x][y];
        }
    }
#pragma omp barrier
    compute_gradient(shared_data_.p, shared_data_.gradpx, shared_data_.gradpy, shared_data_.gradp, h, y_start, y_end);
    do {
        local_diff = 0.0;
#pragma omp barrier
        for (int y = y_start + y_start % 2; y < y_end; y+=2) {
            for (int x = 0; x < size_x; x++) {
                pp = shared_data_.p[x][y];
                lp = (x == 0)        ? shared_data_.p[1][y]        : shared_data_.p[x-1][y];
                rp = (x == size_x-1) ? shared_data_.p[size_x-2][y] : shared_data_.p[x+1][y];
                dp = (y == 0)        ? shared_data_.p[x][1]        : shared_data_.p[x][y-1];
                up = (y == size_y-1) ? shared_data_.p[x][size_y-2] : shared_data_.p[x][y+1];
                gp = shared_data_.gradp[x][y];

                gg = shared_data_.gh[x][y];
                lg = (x == 0)        ? shared_data_.gx[0][y]        : shared_data_.gx[x-1][y];
                rg = (x == size_x-1) ? shared_data_.gx[size_x-2][y] : shared_data_.gx[x][y];
                dg = (y == 0)        ? shared_data_.gy[x][0]        : shared_data_.gy[x][y-1];
                ug = (y == size_y-1) ? shared_data_.gy[x][size_y-2] : shared_data_.gy[x][y];

                double sum = shared_data_.p_old[x][y] + tau * gg * F * gp; // F
                sum += tau/xi/xi * gg * f0(pp); // tau/xi^2*g*f(p)
                sum += tau*h_pow2_inv*(lg*lp + dg*dp); // Lp
                sum += tau*h_pow2_inv*(rg*rp + ug*up); // Up
                sum /= (1.0 + tau*h_pow2_inv*(rg+lg+ug+dg));

                local_diff = std::max(local_diff, fabs(pp-sum));
                shared_data_.p[x][y] = sum;
            }
        }
#pragma omp barrier
        for (int y = y_start + (y_start + 1) % 2; y < y_end; y+=2) {
            for (int x = 0; x < size_x; x++) {
                pp = shared_data_.p[x][y];
                lp = (x == 0)        ? shared_data_.p[1][y]        : shared_data_.p[x-1][y];
                rp = (x == size_x-1) ? shared_data_.p[size_x-2][y] : shared_data_.p[x+1][y];
                dp = (y == 0)        ? shared_data_.p[x][1]        : shared_data_.p[x][y-1];
                up = (y == size_y-1) ? shared_data_.p[x][size_y-2] : shared_data_.p[x][y+1];
                gp = shared_data_.gradp[x][y];

                gg = shared_data_.gh[x][y];
                lg = (x == 0)        ? shared_data_.gx[0][y]        : shared_data_.gx[x-1][y];
                rg = (x == size_x-1) ? shared_data_.gx[size_x-2][y] : shared_data_.gx[x][y];
                dg = (y == 0)        ? shared_data_.gy[x][0]        : shared_data_.gy[x][y-1];
                ug = (y == size_y-1) ? shared_data_.gy[x][size_y-2] : shared_data_.gy[x][y];

                double sum = shared_data_.p_old[x][y] + tau * gg * F * gp; // F
                sum += tau/xi/xi * gg * f0(pp); // tau/xi^2*g*f(p)
                sum += tau*h_pow2_inv*(lg*lp + dg*dp); // Lp
                sum += tau*h_pow2_inv*(rg*rp + ug*up); // Up
                sum /= (1.0 + tau*h_pow2_inv*(rg+lg+ug+dg));

                local_diff = std::max(local_diff, fabs(pp-sum));
                shared_data_.p[x][y] = sum;
            }
        }
        shared_data_.conv_diff[tid_] = local_diff;
#pragma omp barrier
#pragma omp single
        {
            double global_diff = shared_data_.conv_diff[0];
            for (int i = 1; i < nthreads_; ++i) {
                global_diff = std::max(global_diff, shared_data_.conv_diff[i]);
            }
            shared_data_.step_end = global_diff < shared_data_.gs_conv_tolerance;
        }
#pragma omp barrier
    } while (not shared_data_.step_end);
}

