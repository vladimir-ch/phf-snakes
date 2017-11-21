**This repository has been archived.**

phf-snakes
==========

Parallel C++ implementation of a phase-field-based active contour model

----------------------------------------------------------------------------------------------------

Introduction
------------

This small program is a parallel C++ implementation of the image segmentation algorithm described in
the paper [M. Beneš, V. Chalupecký, and K. Mikula, Geometrical image segmentation by the Allen-Cahn
equation, Applied Numerical Mathematics, 51 (2004), pp.
187—205](http://dx.doi.org/10.1016/j.apnum.2004.05.001). The parallelization is done via SPMD style
OpenMP. The image is split into blocks and each thread is responsible for updating its block. The
Gauss-Seidel solver uses a simple red-black ordering in the y-direction where all the threads first
update even rows and then odd rows of the image. The program reads and writes images in PNG format.

Compilation
-----------

`phf-snakes` has only a couple of dependencies. These are:

* boost (multi_array, exception)
* libpng and zlib
* OpenMP-capable C++ compiler
* CMake

To compile `phf-snakes`, use the following commands:

```
$ mkdir phf-snakes.rel
$ cd phf-snakes.rel
$ cmake path/to/source/directory
$ make
$ cp -r path/to/source/directory/images .
$ ./phf-snakes
```

Usage
-----

Change parameters by editing `phf-snakes.dat`. Some test images are available in `images/`
directory. Note that for each image the program needs the corresponding outer or inner contour image
file, depending on the sign of F. Output images are saved in `results/` subdirectory. See the source
code and the article for details.
