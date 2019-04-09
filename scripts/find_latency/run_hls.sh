#! /bin/bash
DESIGNNAME=facedetect
DEVICENAME=cycloneV
CYCLE=2000 # unit: 10ps, e.g. 2000 means 20ns

if [ $# -eq 2 ]; then
	CYCLE=$1
	DEVICENAME=$2
elif [ $# -ne 0 ]; then
	echo Please either give 2 arguments, in which the first is clock cycle, and the second is device name, or 0 argument to use the default values.
	exit 1
fi

mkdir -p hls
cp ./src/{define.h,facedetect.h,facedetect.cpp,*dat} ./hls/
if [ $? -ne 0 ]; then
	echo copy ERROR
	exit 1
fi

cd hls/
scpars -DCYSIM ${DESIGNNAME}.cpp
if [ $? -ne 0 ]; then
	echo scpars ERROR
	exit 1
fi

timeout 30m bdltran -c${CYCLE} -s -Zresource_fcnt=GENERATE -Zresource_mcnt=GENERATE -Zdup_reset=YES -Zfolding_sharing=inter_stage -tcio -lb /eda/cwb/cyber_61/LINUX/packages/${DEVICENAME}.BLIB -lfl /eda/cwb/cyber_61/LINUX/packages/${DEVICENAME}.FLIB ${DESIGNNAME}.IFF
if [ $? -ne 0 ]; then
	echo bdltran ERROR
	exit 1
fi

veriloggen ${DESIGNNAME}_E.IFF

cd ..

exit 0