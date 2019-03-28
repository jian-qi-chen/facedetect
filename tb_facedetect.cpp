//============================================================================================
// 
// File Name    : tb_facedetect.cpp
// Description  : Testbench for the face detector 
// Release Date : 25/03/2019
// Author       : Jianqi Chen, Benjamin Carrion Schafer
//
// Revision History
//--------------------------------------------------------------------------------------------
// Date     Version   Author                            Description
//--------------------------------------------------------------------------------------------
//25/03/2019  1.0   UTD DARClab	                        face detector testbench 
//============================================================================================

#include "define.h"
#include "image.h"
#include "tb_facedetect.h"

using namespace std;

void test_FACEDETECT::test_main ()
{

	int flag;

	int mode = 1;
	int i,j,k;
    int face_number, shiftStep;
    float scaleFactor;
    sc_uint<32> input_data_v;
    sc_uint<OUT_BW*4> output_data_v;
    std::vector<MyRect> result;
    FILE *fp;


	printf("-- entering main function --\r\n");

	printf("-- loading image --\r\n");

	flag = readPgm((char *)"Face.pgm", image);//read the .pgm image
	if (flag == -1)
	{
		printf( "Unable to open input image\n");
		sc_stop();
	}

    wait();
	printf("-- sending data --\r\n");
    
    // read from parameter.txt
    fp = fopen("parameter.txt","r");
    if (!fp){
		printf("Unable to open file parameter.txt\n");
		sc_stop();
	}
    fscanf(fp,"%f",&scaleFactor); //first line
    fscanf(fp,"%d",&shiftStep); //second line
    fclose(fp);
    
    scaleFactor_in.write( (sc_ufixed<8,1,SC_RND,SC_SAT>) scaleFactor );
    shiftStep_in.write( (sc_uint<8>) shiftStep );
    
    for(i=0;i<IMAGE_HEIGHT;i++){
        for(j=0;j<IMAGE_WIDTH;j=j+4){//Make sure IMAGE_WIDTH is multiple of 4
            input_data_v = ((sc_uint<8>)image->data[i*IMAGE_WIDTH+j+3],(sc_uint<8>)image->data[i*IMAGE_WIDTH+j+2],(sc_uint<8>)image->data[i*IMAGE_WIDTH+j+1],(sc_uint<8>)image->data[i*IMAGE_WIDTH+j]);
            in_data.write( input_data_v );
            write_signal.write(1);
            wait();
        }
    }
    
    write_signal.write(0);
    
	printf("-- detecting faces --\r\n");    
    while(ready.read()==0)
        wait();
    
    face_number = face_num_out.read();
    read_signal.write(1);
    wait();
    wait();
    
    // write to facenumber.txt
    fp = fopen("facenumber.txt","w");
    if (!fp){
		printf("Unable to open file facenumber.txt\n");
		sc_stop();
	}
    fprintf(fp,"%d\n",face_number); 
    fclose(fp);
    
    printf("tb: face_num_out=%d\n",face_number);
    for(i=0;i<face_number;i++){
        read_signal.write(1);
        output_data_v = out_data.read();
        MyRect r = {(int)output_data_v.range(OUT_BW-1,0), (int)output_data_v.range(2*OUT_BW-1,OUT_BW), (int)output_data_v.range(3*OUT_BW-1,2*OUT_BW), (int)output_data_v.range(4*OUT_BW-1,3*OUT_BW)};
        result.push_back(r);
        wait();
    }
    read_signal.write(0);
    printf("result size: %d\n",result.size());
	
	for(i = 0; i < result.size(); i++ )
	{
		MyRect r = result[i];
		drawRectangle(image, r);
	}

	printf("-- saving output --\r\n");
	flag = writePgm((char *)OUTPUT_FILENAME, image);

	printf("-- image saved --\r\n");

    
	/* delete image and free classifier */
	freeImage(image);
    wait();
    sc_stop();

}

/* draw white bounding boxes around detected faces */
void test_FACEDETECT::drawRectangle(MyImage* image, MyRect r)
{
    int i;
    int col = image->width;

    for (i = 0; i < r.width; i++)
    {
        image->data[col*r.y + r.x + i] = 255;
    }
    for (i = 0; i < r.height; i++)
    {
        image->data[col*(r.y+i) + r.x + r.width] = 255;
    }
    for (i = 0; i < r.width; i++)
    {
        image->data[col*(r.y + r.height) + r.x + r.width - i] = 255;
    }
    for (i = 0; i < r.height; i++)
    {
        image->data[col*(r.y + r.height - i) + r.x] = 255;
    }
}

