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
    sc_signal<sc_ufixed<8,1,SC_RND,SC_SAT> > scaleFactor_in;
    sc_signal<sc_uint<8> > shiftStep_in;
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
    u_FACEDETECT.scaleFactor_in( scaleFactor_in );
    u_FACEDETECT.shiftStep_in( shiftStep_in );
    
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
    test.scaleFactor_in( scaleFactor_in );
    test.shiftStep_in( shiftStep_in );

#ifdef WAVE_DUMP
    // Trace files
    sc_trace_file* trace_file = sc_create_vcd_trace_file("trace_behav");

    // Top level signals
    sc_trace(trace_file, clk, "clk");
    sc_trace(trace_file, rst, "rst");
    sc_trace(trace_file, write_signal, "write_signal");
    sc_trace(trace_file, read_signal, "read_signal");
    sc_trace(trace_file, in_data[0], "in_data_a00");
    sc_trace(trace_file, in_data[1], "in_data_a01");
    sc_trace(trace_file, in_data[2], "in_data_a02");
    sc_trace(trace_file, in_data[3], "in_data_a03");
    sc_trace(trace_file, out_data[0], "out_data_a00");
    sc_trace(trace_file, out_data[1], "out_data_a01");
    sc_trace(trace_file, out_data[2], "out_data_a02");
    sc_trace(trace_file, out_data[3], "out_data_a03");
    sc_trace(trace_file, face_num_out, "face_num_out");
    sc_trace(trace_file, ready, "ready");
    sc_trace(trace_file, scaleFactor_in, "scaleFactor_in");
    sc_trace(trace_file, shiftStep_in, "shiftStep_in");

#endif  // End WAVE_DUMP
    
    
    rst.write(0);
    
    sc_start( 25, SC_NS );
    rst.write(1);
    
    sc_start();

#ifdef WAVE_DUMP
    sc_close_vcd_trace_file(trace_file);
    printf("trace_behav.vcd file generated.\n");
#endif
    
    return 0;
    
};