//============================================================================================
// 
// File Name    : facedetect.cpp
// Description  : Viola-Jones face detector
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

#include "define.h"
#include "facedetect.h"


#ifdef IO
int writetoTLV_int(unsigned int value, char* filename);
int writetoTLV_float(float value, char* filename);

//write the inputs into tlv file every valid cycle.
void facedetect::writeIO(void)
{
    int ret_v;
    
    ret_v = writetoTLV_int( (unsigned int)write_signal.read(), "./tlv/write_signal.tlv" );
    ret_v += writetoTLV_int( (unsigned int)read_signal.read(), "./tlv/read_signal.tlv" );
    ret_v += writetoTLV_int( (unsigned int)in_data.read(), "./tlv/in_data.tlv" );
    ret_v += writetoTLV_float( (float)scaleFactor_in.read(), "./tlv/scaleFactor_in.tlv" );
    ret_v += writetoTLV_int( (unsigned int)shiftStep_in.read(), "./tlv/shiftStep_in.tlv" );

    
    if(ret_v!=0)
        sc_stop();
    
}

int writetoTLV_int(unsigned int value, char* filename)
{
    FILE* fp;
    fp = fopen(filename, "a");
    if (fp == NULL){
        printf("ERROR: unable to open file %s\n",filename);
        return -1;
    }
    
    fprintf(fp, "%u\n", value);
    fclose(fp);
    return 0;
}

int writetoTLV_float(float value, char* filename)
{
    FILE* fp;
    fp = fopen(filename, "a");
    if (fp == NULL){
        printf("ERROR: unable to open file %s\n",filename);
        return -1;
    }
    
    fprintf(fp, "%f\n", value);
    fclose(fp);
    return 0;
}

#endif



const int rectangles_array[34956] = {
    #include "rectangles_array.dat"
};
const int stages_array[25] = {
    #include "stages_array.dat"
};
const int weights_array[8739] = {
    #include "weights_array.dat"
};
const int alpha1_array[2913] = {
    #include "alpha1_array.dat"
};
const int alpha2_array[2913] = {
    #include "alpha2_array.dat"
};
const int tree_thresh_array[2913] = {
    #include "tree_thresh_array.dat"
};
const int stages_thresh_array[25] = {
    #include "stages_thresh_array.dat"
};

/* rounding function */
inline  int  myRound( sc_ufixed<16,12,SC_RND,SC_SAT> value )
{
  return (int)(value + (sc_ufixed<16,12,SC_RND,SC_SAT>)0.5);
}

/*******************************************************
 * Function: detectObjects
 * Description: It calls all the major steps
 ******************************************************/

void facedetect::detectObjects( MySize minSize,
				   sc_ufixed<8,1,SC_RND,SC_SAT> scaleFactor, int minNeighbors, int shift_step)
{

    /* group overlaping windows */
    const sc_ufixed<8,1,SC_RND,SC_SAT> GROUP_EPS = 0.4;
    int y_bias, iter_counter = 0;

    /* scaling factor */
    sc_ufixed<10,5,SC_RND,SC_SAT> factor;

    /* window size of the training set */
    MySize winSize0 = cascadeObj.orig_window_size;

    /* iterate over the image pyramid */
    face_number = 0;
    for( factor = 1; ; factor *= scaleFactor)
    {
        /* iteration counter */
        iter_counter++;

        /* size of the image scaled up */
        MySize winSize = { myRound(winSize0.width*factor), myRound(winSize0.height*factor) };

        /* size of the image scaled down (from bigger to smaller) */
        MySize sz = { ( IMAGE_WIDTH/factor ), ( IMAGE_HEIGHT/factor ) };

        /* break if the scale downed image is smaller than the window */
        if( sz.width < 24 || sz.height < 24 )
            break;
        
        /* if a minSize different from the original detection window is specified, continue to the next scaling */
        if( winSize.width < minSize.width || winSize.height < minSize.height )
            continue;

        /***************************************
        * Compute-intensive step:
        * building image pyramid by downsampling
        * downsampling using nearest neighbor
        **************************************/
        nearestNeighbor( downsample_buffer, sz.width, sz.height);

        /***************************************************
        * Compute-intensive step:
        * At each scale of the image pyramid,
        * compute a new integral and squared integral image
        ***************************************************/
        integralImages(downsample_buffer, int_img_buffer, sq_int_buffer, sz.width, 25);

        /**************************************************
        * Note:
        * Summing pixels within a haar window is done by
        * using four corners of the integral image:
        * http://en.wikipedia.org/wiki/Summed_area_table
        *
        * This function loads does not do computation.
        * The computation is done next in ScaleImage_Invoker
        *************************************************/
        setImageForCascadeClassifier(  int_img_buffer, sq_int_buffer, sz.width);

        for(y_bias=0; y_bias < sz.height-25+1; y_bias++){
            if(y_bias!=0)
                // shift integral image buffer and only update the last row of pixels
                integralmages_lastrow(downsample_buffer, int_img_buffer, sq_int_buffer, sz.width, y_bias);
            
            /****************************************************
            * Process the current scale with the cascaded fitler.
            * The main computations are invoked by this function.
            ***************************************************/
            ScaleImage_Invoker( factor, sz.width, shift_step, y_bias);
        }
        
    } /* end of the factor loop, finish all scales in pyramid*/

    if( minNeighbors != 0)
    {
        groupRectangles( minNeighbors, GROUP_EPS);
    }

}


unsigned int int_sqrt (unsigned int value)
{
    int i;
    unsigned int a = 0, b = 0, c = 0;
    for (i=0; i < (32 >> 1); i++)
    {
        c<<= 2;

        c += value>>30;

        value <<= 2;
        a <<= 1;
        b = (a<<1) | 1;
        if (c >= b)
        {
            c -= b;
            a++;
        }
    }
    return a;
}


void facedetect::setImageForCascadeClassifier( int* sum, int* sqsum, int width)
{
    int i, j, k;
    MyRect equRect;
    int r_index = 0;
    int w_index = 0;
    MyRect tr;

    equRect.x = equRect.y = 0;
    equRect.width = cascadeObj.orig_window_size.width;
    equRect.height = cascadeObj.orig_window_size.height;

    cascadeObj.inv_window_area = equRect.width*equRect.height;

    /****************************************
    * Load the index of the four corners
    * of the filter rectangle
    **************************************/

    /* loop over the number of stages */
    for( i = 0; i < cascadeObj.n_stages; i++ )
    {
        /* loop over the number of haar features */
        for( j = 0; j < stages_array[i]; j++ )
        {
            int nr = 3;
            /* loop over the number of rectangles */
            for( k = 0; k < nr; k++ )
            {
                tr.x = rectangles_array[r_index + k*4];
                tr.width = rectangles_array[r_index + 2 + k*4];
                tr.y = rectangles_array[r_index + 1 + k*4];
                tr.height = rectangles_array[r_index + 3 + k*4];
                if (k < 2)
                {
                    scaled_rectangles_array[r_index + k*4] = width*(tr.y ) + (tr.x ) ;
                    scaled_rectangles_array[r_index + k*4 + 1] = width*(tr.y ) + (tr.x  + tr.width);
                    scaled_rectangles_array[r_index + k*4 + 2] = width*(tr.y  + tr.height) + (tr.x );
                    scaled_rectangles_array[r_index + k*4 + 3] = width*(tr.y  + tr.height) + (tr.x  + tr.width);
                }
                else
                {
                    if ((tr.x == 0)&& (tr.y == 0) &&(tr.width == 0) &&(tr.height == 0))
                    {
                        scaled_rectangles_array[r_index + k*4] = -1 ;//null
                        scaled_rectangles_array[r_index + k*4 + 1] = -1 ;//null
                        scaled_rectangles_array[r_index + k*4 + 2] = -1;//null
                        scaled_rectangles_array[r_index + k*4 + 3] = -1;//null
                    }
                    else
                    {
                        scaled_rectangles_array[r_index + k*4] = width*(tr.y ) + (tr.x ) ;
                        scaled_rectangles_array[r_index + k*4 + 1] = width*(tr.y ) + (tr.x  + tr.width) ;
                        scaled_rectangles_array[r_index + k*4 + 2] = width*(tr.y  + tr.height) + (tr.x );
                        scaled_rectangles_array[r_index + k*4 + 3] = width*(tr.y  + tr.height) + (tr.x  + tr.width);
                    }
                } /* end of branch if(k<2) */
            } /* end of k loop*/
            r_index+=12;
            w_index+=3;
        } /* end of j loop */
    } /* end i loop */
}


/****************************************************
 * evalWeakClassifier:
 * the actual computation of a haar filter.
 * More info:
 * http://en.wikipedia.org/wiki/Haar-like_features
 ***************************************************/
int facedetect::evalWeakClassifier(int variance_norm_factor, int p_offset, int tree_index, int w_index, int r_index )
{

    /* the node threshold is multiplied by the standard deviation of the image */
    int t = tree_thresh_array[tree_index] * variance_norm_factor;

    int sum = (int_img_buffer[scaled_rectangles_array[r_index] + p_offset]
        - int_img_buffer[scaled_rectangles_array[r_index + 1] + p_offset]
        - int_img_buffer[scaled_rectangles_array[r_index + 2] + p_offset]
        + int_img_buffer[scaled_rectangles_array[r_index + 3] + p_offset])
        * weights_array[w_index];

    sum += (int_img_buffer[scaled_rectangles_array[r_index+4] + p_offset]
        - int_img_buffer[scaled_rectangles_array[r_index + 5] + p_offset]
        - int_img_buffer[scaled_rectangles_array[r_index + 6] + p_offset]
        + int_img_buffer[scaled_rectangles_array[r_index + 7] + p_offset])
        * weights_array[w_index + 1];

    if ((scaled_rectangles_array[r_index+8] != -1))//null
        sum += (int_img_buffer[scaled_rectangles_array[r_index+8] + p_offset]
            - int_img_buffer[scaled_rectangles_array[r_index + 9] + p_offset]
            - int_img_buffer[scaled_rectangles_array[r_index + 10] + p_offset]
            + int_img_buffer[scaled_rectangles_array[r_index + 11] + p_offset])
            * weights_array[w_index + 2];

    if(sum >= t)
        return alpha2_array[tree_index];
    else
        return alpha1_array[tree_index];

}

void facedetect::updatePvalue(  int* sum, int* sqsum, int p_offset, int pq_offset, int width)
{
    cascadeObj.p0 = sum[0+p_offset] ;
    cascadeObj.p1 = sum[cascadeObj.orig_window_size.width - 1+p_offset] ;
    cascadeObj.p2 = sum[width*(cascadeObj.orig_window_size.height - 1)+p_offset];
    cascadeObj.p3 = sum[width*(cascadeObj.orig_window_size.height - 1) + cascadeObj.orig_window_size.width - 1+p_offset];
    cascadeObj.pq0 = sqsum[0+pq_offset];
    cascadeObj.pq1 = sqsum[cascadeObj.orig_window_size.width - 1+pq_offset] ;
    cascadeObj.pq2 = sqsum[width*(cascadeObj.orig_window_size.height - 1)+pq_offset];
    cascadeObj.pq3 = sqsum[width*(cascadeObj.orig_window_size.height - 1) + cascadeObj.orig_window_size.width - 1+pq_offset];
}

int facedetect::runCascadeClassifier( MyPoint pt, int start_stage, int width)
{

    int p_offset, pq_offset;
    int i, j;
    unsigned int mean;
    unsigned int variance_norm_factor;
    int haar_counter = 0;
    int w_index = 0;
    int r_index = 0;
    int stage_sum;

    p_offset = pt.y * width + pt.x;
    pq_offset = pt.y * width + pt.x;

    /**************************************************************************
    * Image normalization
    * mean is the mean of the pixels in the detection window
    * cascade->pqi[pq_offset] are the squared pixel values (using the squared integral image)
    * inv_window_area is 1 over the total number of pixels in the detection window
    *************************************************************************/

    updatePvalue( int_img_buffer, sq_int_buffer, p_offset, pq_offset, width);
    
    variance_norm_factor =  (cascadeObj.pq0 - cascadeObj.pq1 - cascadeObj.pq2 + cascadeObj.pq3);
    mean = (cascadeObj.p0 - cascadeObj.p1 - cascadeObj.p2 + cascadeObj.p3);

    variance_norm_factor = (variance_norm_factor*cascadeObj.inv_window_area);
    variance_norm_factor =  variance_norm_factor - mean*mean;

    if( variance_norm_factor > 0 )
        variance_norm_factor = int_sqrt(variance_norm_factor);
    else
        variance_norm_factor = 1;

    /**************************************************
    * The major computation happens here.
    * For each scale in the image pyramid,
    * and for each shifted step of the filter,
    * send the shifted window through cascade filter.
    *
    * Note:
    *
    * Stages in the cascade filter are independent.
    * However, a face can be rejected by any stage.
    * Running stages in parallel delays the rejection,
    * which induces unnecessary computation.
    *
    * Filters in the same stage are also independent,
    * except that filter results need to be merged,
    * and compared with a per-stage threshold.
    *************************************************/
    for( i = start_stage; i < cascadeObj.n_stages; i++ )
    {

        /****************************************************
        * A shared variable that induces false dependency
        *
        * To avoid it from limiting parallelism,
        * we can duplicate it multiple times,
        * e.g., using stage_sum_array[number_of_threads].
        * Then threads only need to sync at the end
        ***************************************************/
        stage_sum = 0;

        for( j = 0; j < stages_array[i]; j++ )
        {
            /**************************************************
            * Send the shifted window to a haar filter.
            **************************************************/
            stage_sum += evalWeakClassifier(variance_norm_factor, p_offset, haar_counter, w_index, r_index);
            haar_counter++;
            w_index+=3;
            r_index+=12;
        } /* end of j loop */

        /**************************************************************
        * threshold of the stage.
        * If the sum is below the threshold,
        * no faces are detected,
        * and the search is abandoned at the i-th stage (-i).
        * Otherwise, a face is detected (1)
        **************************************************************/

        /* the number "0.4" is empirically chosen for 5kk73 */
        if( stage_sum < (sc_ufixed<8,1,SC_RND,SC_SAT>)0.4*stages_thresh_array[i] ){
            return -i;
        } /* end of the per-stage thresholding */
    } /* end of i loop */
    return 1;
}


void facedetect::ScaleImage_Invoker( sc_ufixed<10,5,SC_RND,SC_SAT> factor, int sum_col, int shift_step, int y_bias)
{

    MyPoint p;

    int result;
    int x2, x, step;

    MySize winSize0 = cascadeObj.orig_window_size;
    MySize winSize;

    winSize.width =  myRound(winSize0.width*factor);
    winSize.height =  myRound(winSize0.height*factor);

    /********************************************
    * When filter window shifts to image boarder,
    * some margin need to be kept
    *********************************************/
    x2 = sum_col - winSize0.width;
    p.y = 0;
    
    step = shift_step;

    for( x = 0; x <= x2-1; x += step )
    {
        p.x = x;

        result = runCascadeClassifier( p, 0, sum_col);

        if( result > 0 )
        {
            face_coordinate[face_number][0] = myRound(x*factor);
            face_coordinate[face_number][1] = myRound(y_bias*factor);
            face_coordinate[face_number][2] = winSize.width;
            face_coordinate[face_number][3] = winSize.height;
            if(face_number<MAX_NUM_FACE-1)
                face_number++;
        }
    }
}

/*****************************************************
 * Compute the integral image (and squared integral)
 * Integral image helps quickly sum up an area.
 * More info:
 * http://en.wikipedia.org/wiki/Summed_area_table
 ****************************************************/
void facedetect::integralImages( sc_uint<8> src[IMAGE_HEIGHT][IMAGE_WIDTH], int *sumData, int *sqsumData, int width, int height)
{
    int x, y, s, sq, t, tq;
    unsigned char it;

    for( y = 0; y < height; y++)
    {
        s = 0;
        sq = 0;
        /* loop over the number of columns */
        for( x = 0; x < width; x ++)
        {
            it = src[y][x];
            /* sum of the current row (integer)*/
            s += it;
            sq += it*it;

            t = s;
            tq = sq;
            if (y != 0)
            {
                t += sumData[(y-1)*width+x];
                tq += sqsumData[(y-1)*width+x];
            }
            sumData[y*width+x]=t;
            sqsumData[y*width+x]=tq;
        }
    }
}

// shift the buffer and only update the last row
void facedetect::integralmages_lastrow(sc_uint<8> src[IMAGE_HEIGHT][IMAGE_WIDTH], int *sumData, int *sqsumData, int width, int y_bias)
{
    int x, y, s, sq, t, tq;
    unsigned char it;
    
    // shift to the upper row
    for(y=0; y<24; y++){
        for(x=0; x<width; x++){
            sumData[y*width+x] = sumData[(y+1)*width+x];
            sqsumData[y*width+x] = sqsumData[(y+1)*width+x];
        }
    }
    
    // update the last row
    s = 0;
    sq = 0;
    for(x=0; x<width; x++){
        it = src[24+y_bias][x];
        s += it;
        sq += it*it;
        
        t = s;
        tq = sq;
        t += sumData[23*width+x];
        tq += sqsumData[23*width+x];
        
        sumData[24*width+x] = t;
        sqsumData[24*width+x] = tq;
    }
}

/***********************************************************
 * This function downsample an image using nearest neighbor
 * It is used to build the image pyramid
 **********************************************************/
void facedetect::nearestNeighbor ( sc_uint<8> dst[IMAGE_HEIGHT][IMAGE_WIDTH], int width, int height)
{

    int y;
    int j;
    int x;
    int i;
    int w1 = IMAGE_WIDTH;
    int h1 = IMAGE_HEIGHT;
    int w2 = width;
    int h2 = height;

    int rat = 0;
    int x_ratio = (int)((w1<<16)/w2) +1;
    int y_ratio = (int)((h1<<16)/h2) +1;

    for (i=0;i<h2;i++)
    {
        y = ((i*y_ratio)>>16);
        rat = 0;
        for(j=0;j<w2;j++)
        {
            x = (rat>>16);
            dst[i][j] = in_img_buffer[y][x];
            rat += x_ratio;
        }
    }
}



void facedetect::groupRectangles( int groupThreshold, sc_ufixed<8,1,SC_RND,SC_SAT> eps)
{
    if( groupThreshold <= 0 || face_number==0 )
        return;

    int labels[MAX_NUM_FACE];

    int nclasses = partition(labels, eps);
    
    MyRect rrects[MAXLABELS];
    int rweights[MAXLABELS];
    
    int i, j, nlabels = face_number;

    for( i = 0; i < nclasses; i++ )
    {
        rrects[i].x = 0;
        rrects[i].y = 0;
        rrects[i].width = 0;
        rrects[i].height = 0;
        rweights[i]=0;
    }
    
    for( i = 0; i < nlabels; i++ )
    {
        int cls = labels[i];
        rrects[cls].x += face_coordinate[i][0];
        rrects[cls].y += face_coordinate[i][1];
        rrects[cls].width += face_coordinate[i][2];
        rrects[cls].height += face_coordinate[i][3];
        rweights[cls]++;
    }
    
    for( i = 0; i < nclasses; i++ )
    {
        MyRect r = rrects[i];
        sc_ufixed<10,1,SC_RND,SC_SAT> s = 1.0/rweights[i];
        rrects[i].x = myRound(r.x*s);
        rrects[i].y = myRound(r.y*s);
        rrects[i].width = myRound(r.width*s);
        rrects[i].height = myRound(r.height*s);
    }

    face_number=0;

    for( i = 0; i < nclasses; i++ )
    {
        MyRect r1 = rrects[i];
        int n1 = rweights[i];
        if( n1 <= groupThreshold )
            continue;
        /* filter out small face rectangles inside large rectangles */
        for( j = 0; j < nclasses; j++ )
        {
            int n2 = rweights[j];
            /*********************************
            * if it is the same rectangle, 
            * or the number of rectangles in class j is < group threshold, 
            * do nothing 
            ********************************/
            if( j == i || n2 <= groupThreshold )
                continue;
            
            MyRect r2 = rrects[j];

            int dx = myRound( r2.width * eps );
            int dy = myRound( r2.height * eps );

            if( i != j &&
                r1.x >= r2.x - dx &&
                r1.y >= r2.y - dy &&
                r1.x + r1.width <= r2.x + r2.width + dx &&
                r1.y + r1.height <= r2.y + r2.height + dy &&
                (n2 > ( (3>n1) ? 3 : n1 ) || n1 < 3) )
                break;
        }

        if( j == nclasses )
        {
            face_coordinate[face_number][0] = r1.x;
            face_coordinate[face_number][1] = r1.y;
            face_coordinate[face_number][2] = r1.width;
            face_coordinate[face_number][3] = r1.height;
            if(face_number<MAX_NUM_FACE-1)
                face_number++;
        }
    }
}

int facedetect::partition(int* labels, sc_ufixed<8,1,SC_RND,SC_SAT> eps)
{
    int i, j;
    int N = face_number;

    const int _PArent=0;
    const int _RAnk=1;

    int nodes[MAX_NUM_FACE][2];
    
    /* The first O(N) pass: create N single-vertex trees */
    for(i = 0; i < N; i++)
    {
        nodes[i][_PArent]=-1;
        nodes[i][_RAnk] = 0;
    }

    /* The main O(N^2) pass: merge connected components */
    for( i = 0; i < N; i++ )
    {
        int root = i;

        /* find root */
        while( nodes[root][_PArent] >= 0 )
        root = nodes[root][_PArent];

        for( j = 0; j < N; j++ )
        {
            if( i == j || !predicate(eps, face_coordinate[i], face_coordinate[j]))
                continue;
            int root2 = j;

            while( nodes[root2][_PArent] >= 0 )
            root2 = nodes[root2][_PArent];

            if( root2 != root )
            {
                /* unite both trees */
                int rank = nodes[root][_RAnk], rank2 = nodes[root2][_RAnk];
                if( rank > rank2 )
                    nodes[root2][_PArent] = root;
                else
                {
                    nodes[root][_PArent] = root2;
                    nodes[root2][_RAnk] += rank == rank2;
                    root = root2;
                }

                int k = j, parent;

                /* compress the path from node2 to root */
                while( (parent = nodes[k][_PArent]) >= 0 )
                {
                    nodes[k][_PArent] = root;
                    k = parent;
                }

                /* compress the path from node to root */
                k = i;
                while( (parent = nodes[k][_PArent]) >= 0 )
                {
                    nodes[k][_PArent] = root;
                    k = parent;
                }
            }
        }
    }

    int nclasses = 0;

    for( i = 0; i < N; i++ )
    {
        int root = i;
        while( nodes[root][_PArent] >= 0 )
            root = nodes[root][_PArent];
        /* re-use the rank as the class label */
        if( nodes[root][_RAnk] >= 0 )
            nodes[root][_RAnk] = ~nclasses++;
        labels[i] = ~nodes[root][_RAnk];
    }

    return nclasses;
}

int myAbs(int n)
{
  if (n >= 0)
    return n;
  else
    return -n;
}

int facedetect::predicate(sc_ufixed<8,1,SC_RND,SC_SAT> eps, sc_uint<OUT_BW> r1[4], sc_uint<OUT_BW> r2[4])
{
    sc_ufixed<16,8,SC_RND,SC_SAT> delta = (sc_ufixed<16,8,SC_RND,SC_SAT>)0.5*eps*(((r1[2]>r2[2]) ? r2[2] : r1[2]) + ((r1[3]>r2[3]) ? r2[3] : r1[3]));
    return myAbs(r1[0] - r2[0]) <= delta &&
        myAbs(r1[1] - r2[1]) <= delta &&
        myAbs(r1[0] + r1[2] - r2[0] - r2[2]) <= delta &&
        myAbs(r1[1] + r1[3] - r2[1] - r2[3]) <= delta;
}

void facedetect::detection_main()
{
    int i,j;
    sc_uint<32> read_data;
    sc_uint<OUT_BW*4> write_data;

    cascadeObj.n_stages=25;                   //number of strong classifier stages
    cascadeObj.orig_window_size.height = 24;  //original window height
    cascadeObj.orig_window_size.width = 24;   //original window width 
    minNeighbours = 1;
    minSize.height = 20;
    minSize.width = 20;
    
    ready.write(0);
#ifdef IO
    system("mkdir -p tlv && rm ./tlv/*");
    writeIO();
#endif
    wait();
    
    while(1){
        i = 0;
        
/* Cyber scheduling_block */
{
        while(i < IMAGE_HEIGHT){
            j = 0;
            while(j < IMAGE_WIDTH){
                if(write_signal.read()==1){
                    read_data = in_data.read();
                    in_img_buffer[i][j] = read_data.range(7,0);
                    wait();
                    in_img_buffer[i][j+1] = read_data.range(15,8);
                    wait();
                    in_img_buffer[i][j+2] = read_data.range(23,16);
                    wait();
                    in_img_buffer[i][j+3] = read_data.range(31,24);
                    
                    j += 4;
                }
                #ifdef IO
                writeIO();
                #endif
                wait();
            }
            i += 1;
        }       
}

        scaleFactor = scaleFactor_in.read();
        shiftStep = shiftStep_in.read();
        detectObjects(minSize, scaleFactor, minNeighbours, shiftStep);
        
        ready.write(1);
        face_num_out.write(face_number);
        #ifdef IO
        writeIO();
        #endif
        wait();
        
        i = 0;
/* Cyber scheduling_block */
{
        while(i < face_number){
            if(read_signal.read()==1){
                out_data.write( (face_coordinate[i][3],face_coordinate[i][2],face_coordinate[i][1],face_coordinate[i][0]) );
                i++;
            }
            #ifdef IO
            writeIO();
            #endif
            wait();
        }       
}            
        ready.write(0);
        #ifdef IO
        writeIO();
        #endif
        wait();
    }
}


/* End of file. */
