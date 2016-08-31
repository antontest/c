#/*************************************************************        
#FileName : makefile   
#FileFunc : Linux编译链接源程序,生成库
#Version  : V0.1        
#Author   : Antonio
#Date     : 2016-03-24   
#Descp    : Linux下makefile模板       
#*************************************************************/     
# target
TARGET_NAME= libsem.a
TARGET_PATH= .
TARGET=$(TARGET_PATH)/$(TARGET_NAME)

# include
INCLUDE_PATH = . ../../../incs/

# output dir
OUTDIR = .

#install path 
USR_LIB_PATH= .

# Make command to use for dependencies
MAKE = make
RM = rm
MKDIR = mkdir
CC = gcc
XX = g++

# source of .c and .o
SRC_PATH = .
CSRC = $(wildcard $(addsuffix /*.c,$(SRC_PATH)))
CPPSRC = $(wildcard $(addsuffix /*.cpp,$(SRC_PATH)))
COBJ = $(patsubst %.c,${OUTDIR}/%.o,$(notdir $(CSRC)))
CPPOBJ = $(patsubst %.cpp,${OUTDIR}/%.o,$(notdir $(CPPSRC)))

# deal with cpp
ifneq "$(CPPOBJ)" ""
CFLAGS += -lstdc++
endif

# dependent files .d
CDEF = $(patsubst %.c,${OUTDIR}/%.d,$(notdir $(CSRC)))
CPPDEF = $(patsubst %.cpp,${OUTDIR}/%.d,$(notdir $(CPPSRC)))

# Warning
WARNING = -Wall -Wno-unused -Wno-format
CFLAGS += $(WARNING) -O2 -Werror

# complie
INC = $(addprefix -I ,$(INCLUDE_PATH))
COMPILE = $(CFLAGS) $(INC) -c $< -o $@ #$(OUTDIR)/$(*F).o

# compile library
LINK = ar -rs $@ $(COBJ) $(CPPOBJ)

# make depend
MAKEDEPEND = gcc -MM -MT

# header and target LINK
CUR_DIR_PATH=$(shell pwd)
CUR_DIR=$(shell basename `pwd`)
LN_FROM_INC=$(CUR_DIR_PATH)
LN_FROM_TARGET=$(CUR_DIR_PATH)/$(TARGET)
LN_TO_INC=../../../incs/$(CUR_DIR)
LN_TO_TARGET=../../../libs/$(TARGET)

all:$(TARGET)
$(OUTDIR) :  
	-if test -n "$(OUTDIR)" ; then $(MKDIR) -p $(OUTDIR) ; fi
$(CDEF) : $(OUTDIR)/%.d : $(SRC_PATH)/%.c $(OUTDIR)
	$(MAKEDEPEND) $(<:.c=.o) $< > $@
$(CPPDEF) : $(OUTDIR)/%.d : ${SRC_PATH}/%.cpp $(OUTDIR)
	$(MAKEDEPEND) $(<:.cpp=.o) $< > $@
depend :
	-rm -f $(CDEF)
	-rm -f $(CPPDEF)
	$(MAKE) $(CDEF)
	$(MAKE) $(CPPDEF)
$(COBJ) : $(OUTDIR)/%.o : $(SRC_PATH)/%.c
	$(CC) $(COMPILE)
$(CPPOBJ) : $(OUTDIR)/%.o : $(SRC_PATH)/%.cpp
	$(XX) $(COMPILE)
$(TARGET) : $(OUTDIR) $(COBJ) $(CPPOBJ)
	$(LINK)
	# -test ! -L $(LN_TO_INC) && ln -sf $(LN_FROM_INC) $(LN_TO_INC)
	# -test ! -L $(LN_TO_TARGET) && ln -sf $(LN_FROM_TARGET) $(LN_TO_TARGET)
	-$(shell test ! -L $(LN_TO_INC) && ln -sf $(LN_FROM_INC) $(LN_TO_INC))
	-$(shell test ! -L $(LN_TO_TARGET) && ln -sf $(LN_FROM_TARGET) $(LN_TO_TARGET))
-include $(CDEF)
-include $(CPPDEF)

PHONY = rebuild clean cleanall install
.PHONY : $(PHONY)
# Rebuild this project
rebuild : cleanall all
#
# Clean this project
clean :
	-$(RM) -f $(COBJ) $(CPPOBJ)
	-$(RM) -f $(TARGET)
	-$(RM) -f $(LN_TO_INC)
	-$(RM) -f $(LN_TO_TARGET)	

# Clean this project and all dependencies
cleanall : clean
	-$(RM) $(CDEF) $(CPPDEF)

# Install lib or share
install:
	-install -p -D -m 0444 $(TARGET) $(USR_LIB_PATH)/$(TARGET)
uninstall:
	-$(RM) -f $(USR_LIB_PATH)/$(TARGET)
