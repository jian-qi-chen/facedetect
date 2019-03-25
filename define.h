//============================================================================================
// 
// File Name    : define.h
// Description  : Main definition header
// Release Date : 25/03/2019
// Author       : Jianqi Chen, Benjamin Carrion Schafer
//
// Revision History
//--------------------------------------------------------------------------------------------
// Date       Version   Author                       Description
//--------------------------------------------------------------------------------------------
//25/03/2019  1.0       UTD DARClab	                 face detector main definition header 
//============================================================================================

#ifndef DEFINE_H
#define DEFINE_H

#define SC_INCLUDE_FX

#include "systemc.h"
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMAGE_WIDTH 360
#define IMAGE_HEIGHT 240
#define PGM_MAXGRAY 255
#define MAX_NUM_FACE 128
#define MAXLABELS 30
#define OUT_BW 10
// #define INT_IMG_BW 26 //integral image bitwidth
// #define INT_IMG_SQ_BW 32 // squared integral image bitwidth
// #define SCALE_FACTOR 1.2
// #define MAX_ITER 13 // MAX_ITER = round down to integer( log_{SCALE_FACTOR}{ min(IMAGE_HEIGHT,IMAGE_WIDTH)/24 } ) + 1

#define INPUT_FILENAME "Face.pgm"
#define OUTPUT_FILENAME "Output.pgm"


typedef struct MyPoint
{
    int x;
    int y;
}
MyPoint;

struct MySize
{
    int width;
    int height;
};

struct MyRect
{
    int x;
    int y;
    int width;
    int height;
};

struct MyImage
{
	int width;
	int height;
	int maxgrey;
	unsigned char* data;
	int flag;
};

struct MyIntImage
{
	int width;
	int height;
	int* data;
	int flag;
};

struct myCascade
{
    // number of stages (22)
    int  n_stages;
    
    // size of the window used in the training set (20 x 20)
    MySize orig_window_size;

    int inv_window_area;
   
    // pixel value of the corner of the actual detection window
    int pq0, pq1, pq2, pq3; //sqsum: squared integral image
    int p0, p1, p2, p3; //sum: integral image

} ;

#endif
