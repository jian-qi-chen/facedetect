//============================================================================================
// 
// File Name    : image.h
// Description  : image management function declaration
// Release Date : 25/03/2019
// Author       : Francesco Comaschi,
//                Jianqi Chen, Benjamin Carrion Schafer
//
// Revision History
//--------------------------------------------------------------------------------------------
// Date     Version   Author                            Description
//--------------------------------------------------------------------------------------------
//12/11/2012  1.0   Francesco Comaschi, TU Eindhoven    Functions to manage .pgm images and integral images
//25/03/2019  1.1   UTD DARClab	                        Add a function for debug 
//============================================================================================
 
#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "define.h"

int readPgm(char *fileName, MyImage* image);
int writePgm(char *fileName, MyImage* image);
int cpyPgm(MyImage *src, MyImage *dst);
void createImage(int width, int height, MyImage *image);
void createSumImage(int width, int height, MyIntImage *image);
int freeImage(MyImage* image);
int freeSumImage(MyIntImage* image);
void setImage(int width, int height, MyImage *image);
void setSumImage(int width, int height, MyIntImage *image);
int checkImg(sc_uint<8> buffer[IMAGE_HEIGHT][IMAGE_WIDTH], int width, int height);

#endif
