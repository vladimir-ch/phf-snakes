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

#include "utils.h"

#include <ctime>
#include <sys/stat.h>
#include <unistd.h>

std::string jobid () {
    time_t curTime = std::time(0);
    tm * t = localtime(&curTime);
    char hostname[256];
    gethostname(hostname, 256);
    std::string id(hostname);
    id += "_" + to_string(t->tm_year + 1900) + to_string(t->tm_mon+1, 2);
    id += to_string(t->tm_mday, 2) + "_" + to_string(t->tm_hour, 2);
    id += to_string(t->tm_min, 2) + to_string(t->tm_sec, 2);
    return id;
}

std::string prepare_output_directory (std::string const& problem_name) {
    std::string path = "./results/";
    mkdir(path.c_str(), 0700);
    path += problem_name + "/";
    mkdir(path.c_str(), 0700);
    path += jobid() + "/";
    mkdir(path.c_str(), 0700);
    return path;
}

