#! /usr/bin/env python3
import os, sys, getopt, json, time, re, math
import matplotlib.pyplot as plt

testcase_num = 8
hls_flag = 0 # 1 means force to re-high-level synthesize
mi_flag = 0 # manualinput flag
hls_cycle = '2000'
hls_device = 'cycloneV'

ALUT_PWR = 0.00326 # 1 ALUT consumes 0.00326 mW
REG_PWR = 0.00238 # 1 Register(FF) consumes 0.00238mW
RAM10K_PWR = 0.473 # 1 M10K block, including 10K bits
DSP9_PWR = 0.255
DSP18_PWR = 0.526
DSP27_PWR = 1.134

def usage():
    print('This program will run high-level synthesis and cycle-accurate simulation to find out the real latency of the face detector. The SystemC source code is in ./src/, test vector of difference test images are in ./tlv_data/. Apart from test vectors, different shiftStep(ss) and scaleFactor(sf) will also be tested. The face detected rate under different ss and sf can be obtained through pure systemC simulation which can be performed in another python program "facedetected_vs_alg_param.py", and the results are stored in the form of json file. These json files (2D lists) are the inputs of this program and stored in ./json/: shiftStep(X), scaleFactor(Y). facedetected(results). The results of this program is a graph of facedetected vs. latency, which will be stored as a png image, and a csv file will be generated as well. The latency generated is the average latency of testcase_num test cases.\n\n')
    print('To run the program:\n')
    print('\t./find_latency.py\n\n')
    print('options:')
    print('\t-h,--help: show usage')
    print('\t-s: only run one set of test vectors (tlv0), reducing running time.')
    print('\t-f: force to re-high-level synthesize.')
    print('\t-c <N>: set N (10ps) as the target clock cycle of HLS, e.g. -c 2000 means the clock cycle is 20ns (default). This option needs to be used together with "-f" to be effective')
    print('\t-d <string>: "string" is the name of target device of HLS, default: cycloneV. This option needs to be used together with "-f" to be effective') 
    print('\t-x: Plot latency vs. face detection accuracy (precision) from the json files in ./json/ that generated last time without runing the simulation again. (This option overwrites any other options.)')
    print('\t--manualinput <N>,<M>: Using this option make the program to not read input files in ./json/ and run only one round of simulation, in which N is assigned to shiftStep, and M is assigned to scaleFactor. Please note that there is a comma and NO space between N and M.\n')
    print('Before running the program, please check the images in ./tlv_data/ and the global variable "testcase_num" in this python file first!\n')
    
def main(argv):
    global testcase_num, hls_flag, mi_flag, hls_cycle, hls_device
    mi_shiftStep = 0
    mi_scaleFactor = 0
    
    try:
        opts, args = getopt.getopt(argv,'hsfc:d:x',['help','manualinput='])
    except getopt.GetoptError:
        usage()
        sys.exit(2)
        
    for opt, arg in opts:
        if opt in ('-h','--help'):
            usage()
            sys.exit(0)
        elif opt == '-s':
            testcase_num = 1
        elif opt == '-f':
            hls_flag = 1
        elif opt == '-c':
            hls_cycle = arg
        elif opt == '-d':
            hls_device = arg
        elif opt == '--manualinput':
            mi_flag = 1
            mi_shiftStep = int( arg.split(',')[0] )
            mi_scaleFactor = float(arg.split(',')[1] )
        elif opt == '-x':
            DirectPlot()
            sys.exit(0)
        else:
            usage()
            sys.exit(2)
            
    FindLatency(mi_shiftStep, mi_scaleFactor)
    sys.exit(0)
    
def FindLatency(mi_shiftStep, mi_scaleFactor):
    os.system('mkdir -p src json tlv_data')
    os.system('cp -r ../testimages/* ./tlv_data/')
    os.system('cp ../../{facedetect.cpp,facedetect.h,define.h,*.dat} ./src/')
    if not ( mi_flag or ( os.path.isfile('./json/shiftStep') and os.path.isfile('./json/scaleFactor') and os.path.isfile('./json/facedetected') ) ):
        ret_v = os.system('cp ../facedetected_vs_ss_sf/json/{facedetected,scaleFactor,shiftStep} ./json/')
        if ret_v!=0:
            print('json file not found.You have to run facedetected_vs_alg_param.py first.\n')
            sys.exit(2)
            
    with open('FindLatency_log.txt','w') as f:
        f.write('Starting find latency program at '+time.strftime("%a,%d %b %Y %H:%M:%S", time.localtime())+'\n\n')
        
    os.system('mkdir -p tlv')
    os.system('echo 1 > ./tlv/read_signal.tlv')
    if not os.path.isfile('./hls/facedetect_C.IFF') or hls_flag:
        # High-level synthesis
        with open('FindLatency_log.txt','a') as f:
            f.write('Performing high-level synthesis...target clock cycle = '+hls_cycle+' *100ps, target device = '+hls_device+'\n')
        ret_v = os.system('bash run_hls.sh '+hls_cycle+' '+hls_device )
        if ret_v!=0:
            with open('FindLatency_log.txt','a') as f:
                f.write('Errors occur during high-level synthesis.(maybe just timeout, please check HLS log)\n')
            print('Errors occur during high-level synthesis.\n')
            sys.exit(2)
    else:
        with open('FindLatency_log.txt','a') as f:
            f.write('High-level synthesis results founded...\n')
    
    if not mi_flag:
        with open('./json/facedetected','r') as f:
            facedetected_list = json.load(f)
        with open('./json/shiftStep','r') as f:
            shiftStep_list = json.load(f)
        with open('./json/scaleFactor','r') as f:
            scaleFactor_list = json.load(f)
        with open('FindLatency_log.txt','a') as f:
            f.write('facedetected, shiftStep, scaleFactor json files loaded.\n')
            
    else:
        shiftStep_list = [ [mi_shiftStep] ]
        scaleFactor_list = [ [mi_scaleFactor] ]
        with open('FindLatency_log.txt','a') as f:
            f.write('Manually set the parameters shiftStep ='+str(mi_shiftStep)+', scaleFactor = '+str(round(mi_scaleFactor,2))+'\n')
        
    ss_len = len(shiftStep_list)
    sf_len = len(shiftStep_list[0])
    latency = [ [] for i in range(ss_len)] #unit: ms

    with open('FindLatency_log.txt','a') as f:
        f.write('In total '+str(ss_len*sf_len)+' scenarios will be tested. For each scenario '+str(testcase_num)+' simulations (test images) will be run.\n')
        f.write('Multiple simulations starting at '+time.strftime("%a,%d %b %Y %H:%M:%S", time.localtime())+'\n\n')
        f.write('Total '+str(ss_len*sf_len)+' rounds of simulations:\n')
    
    sim_round = 0
    for i in range(ss_len):
        for j in range(sf_len):
            shiftStep = shiftStep_list[i][j]
            scaleFactor = scaleFactor_list[i][j]
            os.system('echo '+str(int(shiftStep))+' > ./tlv/shiftStep_in.tlv')
            os.system('echo '+str(float(scaleFactor))+' > ./tlv/scaleFactor_in.tlv')
            
            latency_sum = 0
            for k in range(testcase_num):
                ret_v = os.system('cp ./tlv_data/tlv'+str(k)+'/*.tlv ./tlv/')
                if ret_v!=0:
                    print('Errors occur when copying the tlv data.\n')
                    with open('FindLatency_log.txt','a') as f:
                        f.write('Errors occur when copying the tlv data.\n')
                    sys.exit(2)
                
                # Cycle-accurate simulation
                ret_v = os.system('bash run_simulation.sh')
                if ret_v!=0:
                    print('Errors occur during cycle-accurate simulation.\n')
                    with open('FindLatency_log.txt','a') as f:
                        f.write('Errors occur during cycle-accurate simulation.\n')
                    sys.exit(2)
                    
                with open('./sim/simlog.txt','r') as f:
                    latency_str = f.read().splitlines()[-1].split(' ')[0] #in ns
                    
                latency_sum += ( int(latency_str)-int(hls_cycle)/100*20 )/1000000 # subtract the extra 20 clock cycles, and convert ns to ms
            
            latency[i].append( latency_sum/testcase_num )
            sim_round += 1
            with open('FindLatency_log.txt','a') as f:
                f.write('round '+str(sim_round)+' finished\n')
    
    with open('FindLatency_log.txt','a') as f:
        f.write('\nSimulations Ended at '+time.strftime("%a,%d %b %Y %H:%M:%S", time.localtime())+' \n')
     
    # write to CSV
    design_power = CalculatePower() #assume the power is fixed
    energy_list = [ [] for i in range(ss_len)]
    with open('FindLatency_results.csv','w') as f:
        f.write('index, shift step, scale factor, latency(ms), energy(mJ), accuracy\n') 
        index = 0
        for i in range(ss_len):
            for j in range(sf_len):
                f.write(str(index)+',')
                f.write(str(int(shiftStep_list[i][j]))+',')
                f.write(str(round(scaleFactor_list[i][j],2))+',')
                f.write(str(latency[i][j])+',')
                energy = design_power * latency[i][j]/1000
                energy_list[i].append(energy)
                f.write(str(energy)+',')
                if not mi_flag:
                    f.write(str(round(facedetected_list[i][j],3))+'\n')
                else:
                    f.write('-\n')
                index += 1
        
    # write to json
    with open('./json/latency','w') as f:
        json.dump(latency, f)
    with open('./json/energy','w') as f:
        json.dump(energy_list, f)
        
    if not mi_flag:
        # Plot latency vs. face detection accuracy
        latency_1d = sum( latency, [] )
        facedetected_1d = sum( facedetected_list, [] )
        plt.ion() #make the plt.show() non-blocking
        plt.plot(latency_1d, facedetected_1d,'.')
        plt.xlabel('Latency (ms)')
        plt.ylabel('Accuracy (Precision)')
        plt.savefig('./latency_vs_accuracy.png', dpi=300)
        plt.show()
        
        plt.clf()
        # Plot energy vs. face detection accuracy
        energy_1d = sum( energy_list, [] )
        plt.plot(energy_1d, facedetected_1d,'.')
        plt.xlabel('Energy (mJ)')
        plt.ylabel('Accuracy (Precision)')
        plt.savefig('./energy_vs_accuracy.png', dpi=300)
        plt.show()
        
def DirectPlot():
    with open('./json/facedetected','r') as f:
        facedetected = json.load(f)
    with open('./json/latency','r') as f:
        latency = json.load(f)
    with open('./json/energy','r') as f:
        energy = json.load(f)
        
    latency_1d = sum( latency, [] )
    facedetected_1d = sum( facedetected, [] )
    energy_1d = sum( energy, [] )
    
    plt.plot(latency_1d, facedetected_1d,'.')
    plt.xlabel('Latency (ms)')
    plt.ylabel('Accuracy (Precision)')
    plt.savefig('./latency_vs_accuracy.png', dpi=300)
    plt.show()

    plt.plot(energy_1d, facedetected_1d,'.')
    plt.xlabel('Energy (mJ)')
    plt.ylabel('Accuracy (Precision)')
    plt.savefig('./energy_vs_accuracy.png', dpi=300)
    plt.show()

# Returns Power [mW]
def CalculatePower():
    results = open('./hls/facedetect.CSV').read().splitlines()[1]
    
    #find DSP, RAM, and total Area & Power
    mul9 = 0
    mul18 = 0
    mul27 = 0
    auto_fnct = open('./hls/facedetect-auto.FCNT').read()
    mult_content = re.findall(r'NAME\tmul\d+[\s\S]+AUTO',auto_fnct) #list contains info of multipliers
    for cur_mult in mult_content:
        mul_bw = int( re.findall(r'(?<=NAME\tmul)\d+',cur_mult)[0] ) #bitwidth
        mul_num = int( re.findall(r'(?<=LIMIT\t)\d+',cur_mult)[0] ) #number of multipliers
        if mul_bw <= 9:
            mul9 = mul9 + mul_num
        elif mul_bw <= 18:
            mul18 = mul18 + mul_num
        else:
            mul27 = mul27 + mul_num
            
    alut_count = int( results.split(',')[0] )
    reg_count = int( results.split(',')[3] )
    mem_bits_count = int( results.split(',')[19] )
    
    cur_power = mul9 * DSP9_PWR + mul18 * DSP18_PWR + mul27 * DSP27_PWR + alut_count * ALUT_PWR + reg_count * REG_PWR + math.ceil(mem_bits_count/10000.0) * RAM10K_PWR # power (mW)
    cur_power = cur_power * (2000/int(hls_cycle))        
    return cur_power

            
if __name__ == "__main__":
    main(sys.argv[1:])