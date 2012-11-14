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

#include "array.h"

#include <cmath>

void convolve (array_t const& a, std::vector<double> const& k, array_t& result) {
    array_t b(a);
    int size_x = a.shape()[0];
    int size_y = a.shape()[1];
    int k_size = k.size();
    int k_center = k_size/2;
    for (int jy = 0; jy != size_y; ++jy) {
        for (int jx = k_center; jx != size_x - k_center; ++jx) {
            double sum = 0.0;
            for (int xi = 0; xi != k_size; ++xi) {
                int xx = jx + xi - k_center;
                sum += a[xx][jy] * k[xi];
            }
            b[jx][jy] = sum;
        }
    }
    for (int jy = k_center; jy != size_y - k_center; ++jy) {
        for (int jx = 0; jx != size_x; ++jx) {
            double sum = 0.0;
            for (int yi = 0; yi != k_size; ++yi) {
                int yy = jy + yi - k_center;
                sum += b[jx][yy] * k[yi];
            }
            result[jx][jy] = sum;
        }
    }
}

void compute_gradient (array_t const& src, array_t& dx, array_t& dy, array_t& norm_grad, double h, int y_start, int y_end) {
    int size_x = src.shape()[0];
    int size_y = src.shape()[1];
    for (int y = y_start; y < y_end; y++) {
        for (int x = 0; x < size_x-1; x++) {
            dx[x][y] = (src[x+1][y] - src[x][y])/h;
        }
    }
    for (int y = y_start; y < y_end-1; y++) {
        for (int x = 0; x < size_x; x++) {
            dy[x][y] = (src[x][y+1] - src[x][y])/h;
        }
    }
    for (int y = y_start; y < y_end; y++) {
        for (int x = 0; x < size_x; x++) {
            double r = (x == size_x-1) ? dx[size_x-2][y] : dx[x][y];
            double l = (x == 0)        ? dx[0][y]        : dx[x-1][y];
            double u = (y == size_y-1) ? dy[x][size_y-2] : dy[x][y];
            double d = (y == 0)        ? dy[x][0]        : dy[x][y-1];
            norm_grad[x][y] = std::sqrt(0.5*(r*r+l*l+u*u+d*d));
        }
    }
}

std::vector<double> create_kernel (double sigma, double h) {
    int k_size = (int)ceil(6*sigma/h);
    if (k_size % 2 == 0) {
        k_size++;
    }
    std::vector<double> kernel(k_size);
    int center = k_size/2;
    double twoSigmaPow2 = 2.0*sigma*sigma;
    double coef = 1.0/std::sqrt(M_PI*twoSigmaPow2);
    for (int i = 0; i < k_size; ++i) {
        double x = (i - center)*h;
        kernel[i] = coef*exp(-x*x/twoSigmaPow2);
    }
    double sum = 0.0;
    for (int i = 0; i < k_size; ++i) {
        sum += kernel[i];
    }
    for (int i = 0; i < k_size; ++i) {
        kernel[i] = kernel[i]/sum;
    }
    return kernel;
}


