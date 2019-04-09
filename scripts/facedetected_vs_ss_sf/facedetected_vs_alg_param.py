#! /usr/bin/env python3
import os, sys, getopt, json
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits import mplot3d

face_numbers = [ 8, 7, 7, 3, 3, 3, 3, 2 ] # number of faces in each test image FaceX.pgm
ssmin=1
ssmax=10
sfmin=1.1
sfmax=3
sfnum=10

def usage():
    print('This program finds the number of face detected by the face detector vs. algorithm paramters (shift step and scale factor). A systemC face detector is in ./face_detector/, and a test pgm images are in ./faces/. A graph of results will be shown. min and max value of the shift step (ss) and scale factor (sf) can be specified using options --ssmin, --ssmax, --sfmin, -sfmax. The default values are ssmin=1, ssmax=10, sfmin=1.1, sfmax=3.\n\n')
    print('To run the program with range of shift step [3,20]:\n')
    print('\t./facedetected_vs_alg_param.py --ssmin 3 --ssmax 20\n')
    print('To use the results generated last time in ./tlv/ without running simulation again:')
    print('\t./facedetected_vs_alg_param.py -x\n\n')
    print('Before running the program, please check the face_detector, images and the global variable "face_numbers" in this python file first!\n')

def main(argv):
    global ssmin, ssmax, sfmin, sfmax, sfnum
    
    try:
        opts, args = getopt.getopt(argv,'hx',['help','ssmin=','ssmax=','sfmin=','sfmax=','sfnum='])
    except getopt.GetoptError:
        usage()
        sys.exit(2)
        
    for opt, arg in opts:
        if opt in ('-h','--help'):
            usage()
            sys.exit(0)
        elif opt == '--ssmin':
            ssmin = int(arg)
        elif opt == '--ssmax':
            ssmax = int(arg)
            if ssmax>24 or ssmax<ssmin:
                print('ssmax should be larger than ssmin and smaller than 24(the window side length)\n')
                sys.exit(2)
        elif opt == '--sfmin':
            sfmin = float(arg)
        elif opt == '--sfmax':
            sfmax = float(arg)
            if sfmax<sfmin:
                print('sfmax should be larger than sfmin\n')
                sys.exit(2)
        elif opt == '--sfnum':
            sfnum = int(arg)
        elif opt == '-x':
            DirectPlot()
            sys.exit(0)
        else:
            usage()
            sys.exit(2)
    
    TestFaceDetector()
    

def TestFaceDetector():
    os.system('mkdir -p faces face_detector')
    os.system('cp ../../{*.h,*.cpp,*.dat,Makefile} ./face_detector/')
    os.system('cp ../testimages/{Face0,Face1,Face2,Face3,Face4,Face5,Face6,Face7}.pgm ./faces/')
    if not os.path.isfile('./face_detector/facedetect.exe'):
        ret_v = os.system('cd face_detector && make')
        if ret_v != 0:
            print('Cannot compile the face detector.\n')
            
    total_face = sum(face_numbers)
    
    shift_step_list = np.array( range(ssmin, ssmax+1, 1) )
    scale_factor_list = np.linspace(sfmin, sfmax, sfnum)
    results = np.zeros( (len(shift_step_list), len(scale_factor_list)) ) #np.zeros(row number,column number)
    X = np.zeros( (len(shift_step_list), len(scale_factor_list)) )
    Y = np.zeros( (len(shift_step_list), len(scale_factor_list)) )
    
    for k in range(len(face_numbers)):
        ret_v = os.system('cp ./faces/Face'+str(k)+'.pgm ./face_detector/Face.pgm')
        if ret_v != 0 :
            print('Cannot get the test image Face'+str(k)+'.pgm\n')
            sys.exit(2)
            
        for i, shift_step in enumerate(shift_step_list):
            for j, scale_factor in enumerate(scale_factor_list):
                facenum = RunSim(shift_step,scale_factor)
                if facenum > face_numbers[k]:
                    sys.exit(2)
                    print('Error: false positive exist.')
                results[i][j] = results[i][j] + facenum/total_face

    for i, shift_step in enumerate(shift_step_list):
        for j, scale_factor in enumerate(scale_factor_list):
            X[i][j] = shift_step
            Y[i][j] = scale_factor
            
    os.system('mkdir -p json')
    with open('./json/facedetected','w') as file:
        json.dump(results.tolist(), file)
    with open('./json/shiftStep','w') as file:
        json.dump(X.tolist(), file)
    with open('./json/scaleFactor','w') as file:
        json.dump(Y.tolist(), file)
        
    ax = plt.axes(projection='3d')
    ax.plot_surface(X,Y,results)
    ax.set_xlabel('Shift step')
    ax.set_ylabel('Scale factor')
    ax.set_zlabel('face detected / total face')
    plt.ion()
    plt.show(ax)
    
 
def RunSim(shift_step, scale_factor):
    with open('./face_detector/parameter.txt','w') as file:
        file.write(str(scale_factor)+'\n')
        file.write(str(int(shift_step))+'\n')
        
    ret_v = os.system('cd face_detector && ./facedetect.exe')
    if ret_v != 0:
        print('Runtime error occur in the face detector.')
        sys.exit(2)
        
    with open('./face_detector/facenumber.txt','r') as file:
        face_num = int( file.read().splitlines()[0] )
        
    return face_num

def DirectPlot():
    with open('./json/facedetected','r') as file:
        results = json.load(file)
    with open('./json/shiftStep','r') as file:
        X = json.load(file)
    with open('./json/scaleFactor','r') as file:
        Y = json.load(file)
        
    ax = plt.axes(projection='3d')
    ax.plot_surface(np.array(X),np.array(Y),np.array(results))
    ax.set_xlabel('Shift step')
    ax.set_ylabel('Scale factor')
    ax.set_zlabel('face detected / total face')
    plt.show(ax)

if __name__ == "__main__":
    main(sys.argv[1:])
