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

#include "exceptions.h"
#include "image_io.h"

#include <iostream>
#include <fstream>
#include <png.h>
#include <zlib.h>

std::istream& eatwhite (std::istream& s) {
    char ch;
    while (s.get(ch)) {
        if (!isspace(ch)) {
            s.putback(ch);
            break;
        }
    }
    return s;
}

std::istream& eatcomment (std::istream& s) {
    eatwhite(s);
    char ch;
    if (s.get(ch)) {
        if (ch == '#') {
            while (s.get(ch) && ch != '\n');
        } else {
            s.putback(ch);
        }
    }
    return s;
}

void read_pgm (std::string const& filename, array_t& image) {
    std::ifstream file(filename.c_str());
    if (file.fail())
        BOOST_THROW_EXCEPTION(file_open_error() << string_info(filename));

    std::string header;
    file >> header;
    if (not (header[0] == 'P' && header[1] == '5'))
        BOOST_THROW_EXCEPTION(wrong_signature_error() << string_info(filename));

    int size_x, size_y, maxval;
    eatcomment(file) >> size_x;
    eatcomment(file) >> size_y;
    eatcomment(file) >> maxval;
    image.resize(boost::extents[size_x][size_y]);
    eatwhite(file);
    boost::multi_array<unsigned char,2> raw_data(boost::extents[size_x][size_y]);
    if (not file.read((char*)raw_data.data(), (size_x*size_y)))
        BOOST_THROW_EXCEPTION(file_read_error() << string_info(filename));
    file.close();

    for (int jy = 0; jy != size_y; jy++) {
        for (int jx = 0; jx != size_x; jx++) {
            image[jx][jy] = raw_data[jx][jy]/(double)maxval;
        }
    }
}

void write_pgm(std::string const& filename, array_t const& a) {
    std::ofstream file(filename.c_str());
    if (file.fail())
        BOOST_THROW_EXCEPTION(file_open_error() << string_info(filename));
    int size_x = a.shape()[0];
    int size_y = a.shape()[1];

    file << "P5\n";
    file << size_x << " " << size_y << std::endl;
    file << 255 << std::endl;

    boost::multi_array<unsigned char,2> raw_data(boost::extents[size_x][size_y]);
    for (int jy = 0; jy != size_y; ++jy) {
        for (int jx = 0; jx != size_x; ++jx) {
            if (a[jx][jy] > 1.0) {
                raw_data[jx][jy] = 255;
            } else {
                if (a[jx][jy] < 0.0) {
                    raw_data[jx][jy] = 0;
                } else {
                    raw_data[jx][jy] = 255*a[jx][jy];
                }
            }
        }
    }
    file.write((char const*)raw_data.data(), size_x*size_y);
}

void write_gnuplot(std::string const& filename, array_t const& a) {
    std::ofstream file(filename.c_str());
    if (file.fail())
        BOOST_THROW_EXCEPTION(file_open_error() << string_info(filename));
    file.setf(std::ios::scientific);

    int size_x = a.shape()[0];
    int size_y = a.shape()[1];
    for (int jx = 0; jx != size_x; ++jx) {
        for (int jy = 0; jy != size_y; ++jy) {
            file << std::setw(12) << std::setprecision(5) << a[size_x-jx-1][jy] << std::endl;
        }
        file << std::endl;
    }      
}

void print_png_version_info () {
    std::cout << "Compiled with libpng "<< PNG_LIBPNG_VER_STRING
        << "; using libpng " << png_libpng_ver
        << ".\n";
    std::cout << "Compiled with zlib "<< ZLIB_VERSION
        << "; using zlib " << zlib_version
        << ".\n";
}

void read_png (std::string const& filename, array_t& image) {
    FILE *file = fopen(filename.c_str(), "rb");
    if (file == NULL) {
        BOOST_THROW_EXCEPTION(file_open_error() << string_info(filename));
    }
    
    unsigned char sig[8];
    fread(sig, 1, 8, file);
    if (png_sig_cmp(sig, 0, 8)) {
        BOOST_THROW_EXCEPTION(png_io_error() << string_info("bad signature"));
    }

    png_structp png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        fclose(file);
        BOOST_THROW_EXCEPTION(out_of_memory_error());
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(file);
        BOOST_THROW_EXCEPTION(out_of_memory_error());
    }

    png_infop end_info = png_create_info_struct(png_ptr);
    if (end_info == NULL) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(file);
        BOOST_THROW_EXCEPTION(out_of_memory_error());
    }
    
    png_init_io(png_ptr, file);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);

    png_uint_32 width  = png_get_image_width(png_ptr, info_ptr);
    png_uint_32 height = png_get_image_height(png_ptr, info_ptr);
    int color_type     = png_get_color_type(png_ptr, info_ptr);
    int bit_depth      = png_get_bit_depth(png_ptr, info_ptr);
    int interlace_type = png_get_interlace_type(png_ptr, info_ptr);

    if (interlace_type != PNG_INTERLACE_NONE) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(file);
        BOOST_THROW_EXCEPTION(png_io_error() << string_info("interlaced PNG files not supported"));
    }

    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
    }

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    }

    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png_ptr);
        png_set_strip_alpha(png_ptr);
    }
    
    if (color_type & PNG_COLOR_MASK_ALPHA) {
        png_set_strip_alpha(png_ptr);
    }

    if (bit_depth == 16) {
        png_set_strip_16(png_ptr);
    }

    if (color_type == PNG_COLOR_TYPE_PALETTE || color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
        png_set_rgb_to_gray_fixed(png_ptr, 1, -1, -1);
    }
    
    // int number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    image.resize(boost::extents[width][height]);
    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    if (rowbytes != width) {
        BOOST_THROW_EXCEPTION(png_io_error() << string_info("size mismatch"));
    }
    boost::multi_array<png_byte,1> row(boost::extents[rowbytes]);
    for (int j = 0; j != height; ++j) {
        png_read_row(png_ptr, row.data(), NULL);
        for (int i = 0; i != rowbytes; ++i) {
            image[i][j] = row[i]/255.0;
        }
    }
    png_read_end(png_ptr, end_info);
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    fclose(file);
}

void write_png(std::string const& filename, array_t const& image) {
    FILE* file = fopen(filename.c_str(), "wb");
    if (file == NULL) {
        BOOST_THROW_EXCEPTION(png_io_error());
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        fclose(file);
        BOOST_THROW_EXCEPTION(out_of_memory_error());
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        png_destroy_write_struct(&png_ptr, NULL);
        fclose(file);
        BOOST_THROW_EXCEPTION(out_of_memory_error());
    }

    png_init_io(png_ptr, file);

    png_uint_32 width = image.shape()[0];
    png_uint_32 height = image.shape()[1];

    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);

    boost::multi_array<png_byte,1> row(boost::extents[width]);
    for (int j = 0; j != height; ++j) {
        for (int i = 0; i != row.shape()[0]; ++i) {
            double pixel = image[i][j];
            if (pixel < 0.0) {
                row[i] = 0;
            } else if (pixel > 1.0) {
                row[i] = 255;
            } else {
                row[i] = (png_byte)(image[i][j]*255.0);
            }
        }
        png_write_row(png_ptr, row.data());
    }

    png_write_end(png_ptr, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(file);
}

