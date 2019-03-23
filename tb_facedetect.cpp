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
    int face_number;
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
    
    for(i=0;i<IMAGE_HEIGHT;i++){
        for(j=0;j<IMAGE_WIDTH;j=j+4){//Make sure IMAGE_WIDTH is multiple of 4
            for(k=0;k<4;k++)
                in_data[k].write( image->data[i*IMAGE_WIDTH+j+k] );
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
    
    printf("tb: face_num_out=%d\n",face_number);
    for(i=0;i<face_number;i++){
        read_signal.write(1);
        MyRect r = {(int)out_data[0].read(), (int)out_data[1].read(), (int)out_data[2].read(), (int)out_data[3].read()};
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

