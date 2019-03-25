//============================================================================================
// 
// File Name    : tb_facedetect.h
// Description  : Header of the testbench for the face detector 
// Release Date : 25/03/2019
// Author       : Jianqi Chen, Benjamin Carrion Schafer
//
// Revision History
//--------------------------------------------------------------------------------------------
// Date     Version   Author                            Description
//--------------------------------------------------------------------------------------------
//25/03/2019  1.0   UTD DARClab	                        face detector testbench header 
//============================================================================================

#ifndef TB_FACEDETECT_H_
#define TB_FACEDETECT_H_

#include "define.h"
#include "image.h"
#include "facedetect.h"

SC_MODULE (test_FACEDETECT){
    
    sc_in<bool>     clk;
    sc_in<bool>     rst;
    
    sc_in<sc_uint<OUT_BW> > out_data[4];
    sc_in<sc_uint<8> > face_num_out;
    sc_in<bool> ready;
    sc_out<sc_uint<8> > in_data[4]; //for the case using 32bit bus
    sc_out<bool> write_signal; // burst write valid signal
    sc_out<bool> read_signal; // burst read valid signal
    sc_out<sc_ufixed<8,1,SC_RND,SC_SAT> > scaleFactor_in;
    sc_out<sc_uint<8> > shiftStep_in;
    
    MyImage imageObj;
    MyImage *image = &imageObj;
    
    /* draw white bounding boxes around detected faces */
    void drawRectangle(MyImage* image, MyRect r);
    void test_main ();
    
    SC_CTOR( test_FACEDETECT ){
        SC_CTHREAD(test_main, clk.pos());
        reset_signal_is(rst,false);
    }
};

#endif