TARGET	= facedetect.exe

SRCS	=  \
	main.cpp \
	tb_facedetect.cpp \
	image.cpp \
	facedetect.cpp

HDRS	=  \
	tb_facedetect.h \
	image.h \
	facedetect.h \
	define.h

ifneq (, $(wildcard /bin/uname))
CPU_ARCH = $(shell /bin/uname -m)
endif

ifeq ($(CPU_ARCH),x86_64)
CWB_LIB   = $(CYBER_PATH)/lib64
ARCH_FLAG = 
else
CWB_LIB   = $(CYBER_PATH)/lib
ARCH_FLAG = -m32
endif

ifdef CYBER_SYSTEMC_TARGET_ARCH
CWB_SC_ARCH = $(CYBER_SYSTEMC_TARGET_ARCH)
else
ifeq (, $(findstring Windows,$(OS)))
ifeq ($(CPU_ARCH),x86_64)
CWB_SC_ARCH = linux64
else
CWB_SC_ARCH = linux
endif
else
CWB_SC_ARCH = mingw
endif
endif

ifdef CYBER_SYSTEMC_HOME
CWB_SC_HOME = $(CYBER_SYSTEMC_HOME)
else
CWB_SC_HOME = $(CYBER_PATH)/osci
endif

CWB_SC_INCL = -I"$(CWB_SC_HOME)/include"
CWB_SC_LIB  = "$(CWB_SC_HOME)/lib-$(CWB_SC_ARCH)/libsystemc.a"

EMPTY	=
SPACE	= $(EMPTY) $(EMPTY)
ifeq (, $(wildcard  $(subst $(SPACE),\$(SPACE),$(CYBER_PATH))/gcc/bin))
CC	= g++
RM	= rm
else
CC	= "$(CYBER_PATH)/gcc/bin/g++"
RM	= "$(CYBER_PATH)/gcc/bin/rm"
endif
LINKER	= $(CC)
INCL	= -I"."
CFLAGS	= $(ARCH_FLAG) -O1
LDFLAGS	= $(ARCH_FLAG)
LIBS	= -lm $(CWB_SC_LIB)  -Wl,-rpath="$(CWB_LIB)"
OBJS	= $(notdir $(addsuffix .o,$(basename $(SRCS))))
debug: CFLAGS += -g -DDEBUG
wave: CFLAGS += -DWAVE_DUMP
io: CFLAGS += -DIO

$(TARGET) : $(OBJS)
	$(LINKER) -o "$@" $(LDFLAGS) $(OBJS) $(LIBS)

debug: $(OBJS)
	$(LINKER) -o $(TARGET) $(LDFLAGS) $(OBJS) $(LIBS)
	
wave:  $(OBJS) 
	$(LINKER) -o $(TARGET) $(LDFLAGS) $(OBJS) $(LIBS)
	
io:  $(OBJS) 
	$(LINKER) -o $(TARGET) $(LDFLAGS) $(OBJS) $(LIBS)

image.o: image.cpp $(HDRS)
	$(CC) $(CFLAGS) $(CWB_SC_INCL) $(INCL) -c $< -o $@

main.o: main.cpp $(HDRS)
	$(CC) $(CFLAGS) $(CWB_SC_INCL) $(INCL) -c $< -o $@

facedetect.o: facedetect.cpp $(HDRS)
	$(CC) $(CFLAGS) $(CWB_SC_INCL) $(INCL) -c $< -o $@

tb_facedetect.o: tb_facedetect.cpp $(HDRS)
	$(CC) $(CFLAGS) $(CWB_SC_INCL) $(INCL) -c $< -o $@

clean:
	rm -f *.o Output.pgm facenumber.txt *.vcd $(TARGET)
