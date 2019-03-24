//============================================================================================
// 
// File Name    : main.cpp
// Description  : Face detector top system description
// Release Date : 25/03/2019
// Author       : Jianqi Chen, Benjamin Carrion Schafer
//
// Revision History
//--------------------------------------------------------------------------------------------
// Date       Version   Author                          Description
//--------------------------------------------------------------------------------------------
//25/03/2019  1.0       UTD DARClab	                    Top system declaration 
//============================================================================================

#include "define.h"
#include "facedetect.h"
#include "tb_facedetect.h"

int sc_main(int argc, char** argv)
{
    sc_clock                clk("clk", 25, SC_NS, 0.5, 12.5, SC_NS, true);
    sc_signal<bool>         rst;
    
    sc_signal<bool> write_signal;
    sc_signal<bool> read_signal;
    sc_signal<sc_uint<8> > in_data[4];
    sc_signal<sc_uint<OUT_BW> > out_data[4];
    sc_signal<sc_uint<8> > face_num_out;
    sc_signal<bool> ready;
    int i;
    
    // initialization
    facedetect u_FACEDETECT("face_detect");
    test_FACEDETECT test("test_face_detect");
    
    // connection
    u_FACEDETECT.clk( clk );
    u_FACEDETECT.rst( rst );
    u_FACEDETECT.write_signal( write_signal );
    u_FACEDETECT.read_signal( read_signal );
    for(i=0;i<4;i++)
        u_FACEDETECT.in_data[i]( in_data[i] );
    for(i=0;i<4;i++)
        u_FACEDETECT.out_data[i]( out_data[i] );    
    u_FACEDETECT.face_num_out( face_num_out );
    u_FACEDETECT.ready( ready );
    
    test.clk( clk );
    test.rst( rst );
    test.write_signal( write_signal );
    test.read_signal( read_signal );
    for(i=0;i<4;i++)
        test.in_data[i]( in_data[i] );
    for(i=0;i<4;i++)
        test.out_data[i]( out_data[i] );    
    test.face_num_out( face_num_out );
    test.ready( ready );

    // start simulation
//    sc_start( 25, SC_NS );
    rst.write(0);
    
    sc_start( 25, SC_NS );
    rst.write(1);
    
    sc_start();
    
    return 0;
    
};