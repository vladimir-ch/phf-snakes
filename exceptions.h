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

#ifndef __EXCEPTIONS_H_INCLUDED__
#define __EXCEPTIONS_H_INCLUDED__ 

#include <boost/exception/all.hpp>

typedef boost::error_info <struct tag_string_info, std::string> string_info;

struct shaperec_error : virtual std::exception, virtual boost::exception { };
struct string_to_int_error : virtual shaperec_error {};
struct string_to_double_error : virtual shaperec_error {};
struct out_of_memory_error : virtual shaperec_error {};
struct io_error : virtual shaperec_error {};
struct file_open_error : virtual io_error {};
struct file_read_error : virtual io_error {};
struct png_io_error : virtual io_error {};
struct wrong_signature_error : virtual shaperec_error {};
struct wrong_header_error : virtual shaperec_error {};
struct size_mismatch_error : virtual shaperec_error {};

#endif /* __EXCEPTIONS_H_INCLUDED__ */
