#! /bin/bash
DESIGNNAME=facedetect

mkdir -p sim
cp ./hls/${DESIGNNAME}_C.IFF ./sim/
cp ./tlv/*.tlv ./sim/

cd sim
echo ./${DESIGNNAME}_C.IFF > location.txt 
cmscgen -input=file:transaction -b10 -file_out=transaction -org_type=design -enum_int=NO -reset_bool -out_dir=. -file location.txt
if [ $? -ne 0 ]; then
	exit 1
fi

mkmfsim -GNU  -I"." -target ${DESIGNNAME}_cycle.exe ${DESIGNNAME}_C.MakeInfo
if [ $? -ne 0 ]; then
	exit 1
fi

usleep 10000
THREADNAME=$(grep SC_THREAD ${DESIGNNAME}_C.cpp)
ADDTEXT='\nset_stack_size(67102720);'
sed -i "s/${THREADNAME}/${THREADNAME}${ADDTEXT}/g" ${DESIGNNAME}_C.cpp # to avoid stack overflow

make -f Makefile.GNU
if [ $? -ne 0 ]; then
	exit 1
fi

./${DESIGNNAME}_cycle.exe > simlog.txt
if [ $? -ne 0 ]; then
	exit 1
fi

cd ..
exit 0
