//============================================================================================
// 
// File Name    : image.h
// Description  : image management functions
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

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "image.h"

char* strrev(char* str)
{
	char *p1, *p2;
	if (!str || !*str)
		return str;
	for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
	{
		*p1 ^= *p2;
		*p2 ^= *p1;
		*p1 ^= *p2;
	}
	return str;
}

//int chartoi(const char *string)
//{
//	int i;
//	i=0;
//	while(*string)
//	{
//		// i<<3 is equivalent of multiplying by 2*2*2 or 8
//		// so i<<3 + i<<1 means multiply by 10
//		i=(i<<3) + (i<<1) + (*string - '0');
//		string++;
//
//		// Dont increment i!
//
//	}
//	return(i);
//}

int myatoi (char* string)
{
	int sign = 1;
	// how many characters in the string
	int length = strlen(string);
	int i = 0;
	int number = 0;

	// handle sign
	if (string[0] == '-')
	{
		sign = -1;
		i++;
	}

//	for (i; i < length; i++)
	while(i < length)
	{
		// handle the decimal place if there is one
		if (string[i] == '.')
			break;
		number = number * 10 + (string[i]- 48);
		i++;
	}

	number *= sign;

	return number;
}

void itochar(int x, char* szBuffer, int radix)
{
	int i = 0, n, xx;
	n = x;
	while (n > 0)
	{
		xx = n%radix;
		n = n/radix;
		szBuffer[i++] = '0' + xx;
	}
	szBuffer[i] = '\0';
	strrev(szBuffer);
}


int readPgm(char *fileName, MyImage *image)
{
	FILE *in_file;
	char ch;
	int type;
	char version[3];
	char line[100];
	char mystring [20];
	char *pch;
	int i;
	long int position;

    //changed from 'r' to 'rb' to fix sub and cntrl-z errors since windows reads 'r' as text and 'rb' as binary
    //link to suggestion: http://stackoverflow.com/questions/15874619/reading-in-a-text-file-with-a-sub-1a-control-z-character-in-r-on-windows
    //another link: http://article.gmane.org/gmane.comp.lang.r.devel/33213/match=duncan+murdoch+control+z
	in_file = fopen(fileName, "rb");
	if (in_file == NULL)
	{
		printf("ERROR: Unable to open file %s\n\n", fileName);
		return -1;
	}
	printf("\nReading image file: %s\n", fileName);
	// Determine image type (only pgm format is allowed)*/
	ch = fgetc(in_file);
	if(ch != 'P')
	{
		printf("ERROR: Not valid pgm file type\n");
		return -1;
	}

	ch = fgetc(in_file);


	/*convert the one digit integer currently represented as a character to

         an integer(48 == '0')*/

	type = ch - 48;

	if(type != 5)
	{
		printf("ERROR: only pgm raw format is allowed\n");
		return -1;
	}
	// Skip comments
//	char line[100];
	while ((ch = fgetc(in_file)) != EOF && isspace(ch));
	position = ftell(in_file);


	// skip comments
	if (ch == '#')
		{
			fgets(line, sizeof(line), in_file);
			while ((ch = fgetc(in_file)) != EOF && isspace(ch));//increment steam position until
			position = ftell(in_file);//ftell: get current position is stream
		}

	fseek(in_file, position-1, SEEK_SET);//originally position-1, set to -3, then changed back to -1 after 'rb' change in reading the file

	fgets (mystring , 20, in_file);
	pch = (char *)strtok(mystring," ");
	image->width = atoi(pch);
	pch = (char *)strtok(NULL," ");
	image->height = atoi(pch);
	fgets (mystring , 5, in_file);
	image->maxgrey = PGM_MAXGRAY;
	image->data = (unsigned char*)malloc(sizeof(unsigned char)*(IMAGE_HEIGHT*IMAGE_WIDTH));//new unsigned char[row*col];
	image->flag = 1;
	for(i=0;i<(IMAGE_HEIGHT*IMAGE_WIDTH);i++)
    //for(i=0;i<(738);i++)
	{
		ch = fgetc(in_file);
		image->data[i] = (unsigned char)ch;
	}

	fclose(in_file);
	return 0;
}

int writePgm(char *fileName, MyImage *image)
{
	char parameters_str[5];
	int i;
	const char *format = "P5";
	if (image->flag == 0)
	{
		return -1;
	}

	//changed from 'w' to 'wb' to fix sub and cntrl-z errors since windows writes 'w' as text and 'wb' as binary
    //this is similar to the issue resolved in the readPgm() function
	FILE *fp = fopen(fileName, "wb");
	if (!fp)
	{
		printf("Unable to open file %s\n", fileName);
		return -1;
	}
	fputs(format, fp);
	fputc('\n', fp);

	itochar(image->width, parameters_str, 10);
	fputs(parameters_str, fp);
	parameters_str[0] = 0;
	fputc(' ', fp);

	itochar(image->height, parameters_str, 10);
	fputs(parameters_str, fp);
	parameters_str[0] = 0;
	fputc('\n', fp);

	itochar(image->maxgrey, parameters_str, 10);
	fputs(parameters_str, fp);
	fputc('\n', fp);

	for (i = 0; i < (image->width * image->height); i++)
	{
		fputc(image->data[i], fp);
	}
	fclose(fp);
	return 0;
}

int cpyPgm(MyImage* src, MyImage* dst)
{
	int i = 0;
	if (src->flag == 0)
	{
		printf("No data available in the specified source image\n");
		return -1;
	}
	dst->width = src->width;
	dst->height = src->height;
	dst->maxgrey = src->maxgrey;
	dst->data = (unsigned char*)malloc(sizeof(unsigned char)*(dst->height*dst->width));
	dst->flag = 1;
	for (i = 0; i < (dst->width * dst->height); i++)
	{
		dst->data[i] = src->data[i];
	}
}


void createImage(int width, int height, MyImage *image)
{
	image->width = width;
	image->height = height;
	image->flag = 1;
	image->data = (unsigned char *)malloc(sizeof(unsigned char)*(height*width));
}

void createSumImage(int width, int height, MyIntImage *image)
{
	image->width = width;
	image->height = height;
	image->flag = 1;
	image->data = (int *)malloc(sizeof(int)*(height*width));
}

int freeImage(MyImage* image)
{
	if (image->flag == 0)
	{
		printf("no image to delete\n");
		return -1;
	}
	else
	{
//		printf("image deleted\n");
		free(image->data);
		return 0;
	}
}

int freeSumImage(MyIntImage* image)
{
	if (image->flag == 0)
	{
		printf("no image to delete\n");
		return -1;
	}
	else
	{
//		printf("image deleted\n");
		free(image->data);
		return 0;
	}
}

void setImage(int width, int height, MyImage *image)
{
	image->width = width;
	image->height = height;
}

void setSumImage(int width, int height, MyIntImage *image)
{
	image->width = width;
	image->height = height;
}


// write to a pgm file "test.pgm", in order to check what's inside the buffer
int checkImg(sc_uint<8> buffer[IMAGE_HEIGHT][IMAGE_WIDTH], int width, int height)
{
	char parameters_str[5];
	int i,j;
	const char *format = "P5";

	FILE *fp = fopen("test.pgm", "wb");
	if (!fp)
	{
		printf("Unable to open file test.pgm\n");
		return -1;
	}
	fputs(format, fp);
	fputc('\n', fp);

	itochar(width, parameters_str, 10);
	fputs(parameters_str, fp);
	parameters_str[0] = 0;
	fputc(' ', fp);

	itochar(height, parameters_str, 10);
	fputs(parameters_str, fp);
	parameters_str[0] = 0;
	fputc('\n', fp);

	itochar(PGM_MAXGRAY, parameters_str, 10);
	fputs(parameters_str, fp);
	fputc('\n', fp);

    for (i=0;i<height;i++){
        for(j=0;j<width;j++){
            fputc(buffer[i][j],fp);
        }
    }
	fclose(fp);
	return 0;
}
