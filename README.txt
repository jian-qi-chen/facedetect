This is a synthesizable face detector in SystemC using Viola-Jones algorithm, tested with commercial high-level synthesis tools. It takes in 360 x 240 8-bit grayscale images, and outputs the coordinates of the faces detected. You can complie the code and run pure software simulation using the commands below (check and configure the SystemC path in the Makefile before make):
	$ make
	$ ./facedetect.exe

The files needed for high-level synthesis:
	facedetect.cpp, facedetect.h, define.h, rectangles_array.dat, stages_array.dat, stages_thresh_array.dat, tree_thresh_array.dat, weight_array.dat, alpha1_array.dat, alpha2_array.dat

The code was originally written by Francesco Comaschi in pure C++ (https://sites.google.com/site/5kk73gpu2012/assignment/viola-jones-face-detection). A copy of the original source code is kept in the 'Cosmashi_original' directory.
