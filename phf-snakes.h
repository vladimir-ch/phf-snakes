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

#ifndef __PHF_SNAKES_H_INCLUDED__
#define __PHF_SNAKES_H_INCLUDED__ 

class Phf_snakes {
public:
    Phf_snakes(Phf_snakes_data& shared_data, int tid, int nthreads);
    void solve();

private:
    double f0 (double s) const;
    double compute_difference();
    void step();

    Phf_snakes_data& shared_data_;
    int tid_, nthreads_;
    int size_x, size_y;
    int y_start, y_end;
    double h, h_pow2_inv;
    double xi;
    double tau;
    double a, F;
    double stationarity_test_constant;
    int gs_iterations;
};

#endif /* __PHF_SNAKES_H_INCLUDED__ */
