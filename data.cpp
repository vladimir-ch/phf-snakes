//
//  Copyright (c) 2011-2012 Vladimir Chalupecky
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

#include "data.h"
#include "exceptions.h"
#include "image_io.h"
#include "utils.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>

#include <iostream>

void Phf_snakes_data::read_from_file(std::string const& filename) {
    boost::property_tree::ptree pt;
    read_info(filename, pt);

    P0_filename        = pt.get<std::string>("image");
    F                  = pt.get<double>("F");
    lambda             = pt.get<double>("lambda");
    sigma              = pt.get<double>("sigma");
    C_s                = pt.get<double>("C_s");
    h                  = pt.get<double>("h");
    a                  = pt.get<double>("a");
    save_images        = pt.get<bool>("save_images");
    save_gnuplot       = pt.get<bool>("save_gnuplot");
    save_every_n_step  = pt.get<int>("save_every_n_step");
    check_every_n_step = pt.get<int>("check_every_n_step");
    gs_conv_tolerance  = pt.get<double>("gauss-seidel.tolerance");
    max_gs_iterations  = pt.get<int>("gauss-seidel.max_iterations");
    add_noise          = pt.get<bool>("add_noise");
    
    std::string::size_type name_start = P0_filename.find_last_of('/');
    if (name_start == std::string::npos) {
        name_start = 0;
    } else {
        ++name_start;
    }
    std::string::size_type name_end = P0_filename.find_last_of('.');
    if (name_end == std::string::npos) {
        name_end = P0_filename.size();
    }
    problem_name_ = P0_filename.substr(name_start, name_end-name_start);

    if (F >= 0.0) {
        ini_filename = P0_filename + "-outer-contour.png";
    } else {
        ini_filename = P0_filename + "-inner-contour.png";
    }
    P0_filename = P0_filename + ".png";
    output_path = prepare_output_directory(problem_name_);
    write_info(output_path + "phf-snakes.dat", pt);

    array_t P0;
    read_png(P0_filename, P0);

    size_x = P0.shape()[0];
    size_y = P0.shape()[1];
    p_old.resize (boost::extents[size_x  ][size_y  ]);
    gx.resize    (boost::extents[size_x-1][size_y  ]);
    gy.resize    (boost::extents[size_x  ][size_y-1]);
    gh.resize    (boost::extents[size_x  ][size_y  ]);
    gradpx.resize(boost::extents[size_x-1][size_y  ]);
    gradpy.resize(boost::extents[size_x  ][size_y-1]);
    gradp.resize (boost::extents[size_x  ][size_y  ]);

    std::vector<double> kernel = create_kernel(sigma, h);
    array_t P0_smooth(P0);
    convolve(P0, kernel, P0_smooth);
    compute_gh(P0_smooth);

    read_png(ini_filename, p);
    if (size_x != p.shape()[0] || size_y != p.shape()[1])
        throw size_mismatch_error();

    write_png(output_path + "p-" + to_string(0, 6) + ".png", p);
    write_gnuplot(output_path + "p-" + to_string(0, 6) + ".dat", p);
    write_png(output_path + "P0.png", P0);
    write_gnuplot(output_path + "P0.dat", P0);
    write_png(output_path + "P0_smooth.png", P0_smooth);
    write_gnuplot(output_path + "P0_smooth.dat", P0_smooth);
}

void Phf_snakes_data::print () const {
    using namespace std;
    cout << "------------------------------------------------------------" << endl;
    cout << "input file   = " << P0_filename << endl;
    cout << "contour file = " << ini_filename << endl;
    cout << "h            = " << h << endl;
    cout << "a            = " << a << endl;
    cout << "F            = " << F << endl;
    cout << "C_s          = " << C_s << endl;
    cout << "lambda       = " << lambda << endl;
    cout << "sigma        = " << sigma << endl;
    cout << "G-S convergence tolerance = " << gs_conv_tolerance << endl;
    cout << "maximum G-S iterations    = " << max_gs_iterations << endl;
    cout << "noise added to P0         = " << add_noise << endl;
    cout << "------------------------------------------------------------" << endl;
}

void Phf_snakes_data::compute_gh(array_t const& P0_smooth) {
    compute_gradient(P0_smooth, gradpx, gradpy, gradp, h, 0, size_y);

    for (int y = 0; y < size_y; y++) {
        for (int x = 0; x < size_x-1; x++) {
            gx[x][y] = (g(gradp[x][y]) + g(gradp[x+1][y]))/2.0;
        }
    }

    for (int y = 0; y < size_y-1; y++) {
        for (int x = 0; x < size_x; x++) {
            gy[x][y] = (g(gradp[x][y]) + g(gradp[x][y+1]))/2.0;
        }
    }

    for (int y = 0; y < size_y; y++) {
        for (int x = 0; x < size_x; x++) {
            gh[x][y] = g(gradp[x][y]);
        }
    }
}

