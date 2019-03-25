//============================================================================================
// 
// File Name    : facedetect.h
// Description  : Viola-Jones face detector module declaration
// Release Date : 25/03/2019
// Author       : Francesco Comaschi,
//                Jianqi Chen, Benjamin Carrion Schafer
//
// Revision History
//--------------------------------------------------------------------------------------------
// Date     Version   Author                            Description
//--------------------------------------------------------------------------------------------
//12/11/2012  1.0   Francesco Comaschi, TU Eindhoven    C++ implementation of Viola-Jones algorithm
//25/03/2019  1.1   UTD DARClab	                        Convert it to synthesizable SystemC     
//============================================================================================

#ifndef __HAAR_H__
#define __HAAR_H__

#include "define.h"

SC_MODULE (facedetect){
    sc_in_clk clk;
    sc_in<bool> rst;
    
    sc_in<bool> write_signal; // burst write valid signal
    sc_in<bool> read_signal; // burst read valid signal
    sc_in<sc_uint<8> > in_data[4]; //for the case using 32bit bus
    sc_in<sc_ufixed<8,1,SC_RND,SC_SAT> > scaleFactor_in; // scale factor for image down-sampling
    sc_in<sc_uint<8> > shiftStep_in; // pixel step for window shifting
    sc_out<sc_uint<OUT_BW> > out_data[4]; //(x,y,w,h) coordinate, bitwidth need to be changed for larger image size
    sc_out<sc_uint<8> > face_num_out;
    sc_out<bool> ready;
    
    sc_ufixed<8,1,SC_RND,SC_SAT> scaleFactor; 
    sc_uint<8> shiftStep;
    int minNeighbours;
    MySize minSize; 

    myCascade cascadeObj;
    
    sc_uint<8> in_img_buffer[IMAGE_HEIGHT][IMAGE_WIDTH];
    sc_uint<8> downsample_buffer[IMAGE_HEIGHT][IMAGE_WIDTH];
    int int_img_buffer[IMAGE_HEIGHT * IMAGE_WIDTH]; //integral image buffer
    int sq_int_buffer[IMAGE_HEIGHT * IMAGE_WIDTH]; // squared integral image buffer
    sc_uint<8> face_number;
    sc_uint<OUT_BW> face_coordinate[MAX_NUM_FACE][4]; //store the output coordinates (x,y,w,h)

    int scaled_rectangles_array[34956];

    /* sets images for haar classifier cascade */
    void setImageForCascadeClassifier(  int* sum, int* sqsum, int width, int height);
    
    void updatePvalue(  int* sum, int* sqsum, int p_offset, int pq_offset, int width, int height);

    /* runs the cascade on the specified window */
    int runCascadeClassifier( MyPoint pt, int start_stage, int width, int height);

    void groupRectangles( int groupThreshold, sc_ufixed<8,1,SC_RND,SC_SAT> eps);

    int partition(int* labels, sc_ufixed<8,1,SC_RND,SC_SAT> eps);
    
    int predicate(sc_ufixed<8,1,SC_RND,SC_SAT> eps, sc_uint<OUT_BW> r1[4], sc_uint<OUT_BW> r2[4]);
    
    void ScaleImage_Invoker( sc_ufixed<10,5,SC_RND,SC_SAT> factor, int sum_row, int sum_col, int shift_step);
    
    int evalWeakClassifier(int variance_norm_factor, int p_offset, int tree_index, int w_index, int r_index );
    
    void integralImages( sc_uint<8> src[IMAGE_HEIGHT][IMAGE_WIDTH], int *sumData, int *sqsumData, int width, int height);
    
    void nearestNeighbor ( sc_uint<8> dst[IMAGE_HEIGHT][IMAGE_WIDTH], int width, int height);

    void detectObjects(MySize minSize, sc_ufixed<8,1,SC_RND,SC_SAT> scale_factor, int min_neighbors, int shift_step);

    void detection_main();
            
    SC_CTOR (facedetect){
        SC_CTHREAD (detection_main, clk.pos() );
        reset_signal_is(rst, false);
        sensitive << clk.pos();
    }
		

};
#endif
