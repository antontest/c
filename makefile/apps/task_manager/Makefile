#/*************************************************************        
#FileName : makefile   
#FileFunc : Linux编译链接源程序,生成目标可执行代码      
#Version  : V0.1        
#Author   : Antonio
#Date     : 2016-03-24   
#Descp    : Linux下makefile模板       
#*************************************************************/     
# target
TARGET= task_manager 

# include
INCLUDE_PATH = . ../../incs/

# search the lib which complied by myself
LIB_PATH = ../../libs/
LIB_NAME = udp get_args pool thread event socket utils proc bsem
# other librarys
OTH_LIB =

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
COBJ = $(CSRC:.c=.o)
CPPOBJ = $(CPPSRC:.cpp=.o)
ifneq "$(CPPOBJ)" ""
CFLAGS += -lstdc++
endif

# dependent files .d
CDEF = $(CSRC:.c=.d)
CPPDEF = $(CPPSRC:.cpp=.d)

# Warning
WARNING = -Wall#-Wno-unused -Wno-format
CFLAGS += $(WARNING) -O2 -Werror -g

# complie
INC = $(addprefix -I ,$(INCLUDE_PATH))
COMPILE = $(CFLAGS) $(INC) -c $< -o $@ #$(OUTDIR)/$(*F).o

# Library of compling
LIB_PATH += .
LIBS_PATH = $(addprefix -L ,$(LIB_PATH))
# set lib
LIB = $(addprefix -l,$(LIB_NAME))
LIB := $(LIBS_PATH) $(LIB) $(OTH_LIB)
LINK = $(CC) $(CFLAGS) -o $@ $(COBJ) $(CPPOBJ) $(LIB) #$(OUTDIR)/*.o

# make depend
MAKEDEPEND = gcc -MM -MT

all:$(TARGET)
$(CDEF) : %.d : %.c
	$(MAKEDEPEND) $(<:.c=.o) $< > $@
$(CPPDEF) : %.d : %.cpp
	$(MAKEDEPEND) $(<:.cpp=.o) $< > $@

depend :
	-rm -f $(CDEF)
	-rm -f $(CPPDEF)
	$(MAKE) $(CDEF)
	$(MAKE) $(CPPDEF)

$(COBJ) : %.o : %.c
	$(CC) $(COMPILE)
$(CPPOBJ) : %.o : %.cpp
	$(XX) $(COMPILE)
$(OUTDIR) :  
	if test -n "$(OUTDIR)" ; then $(MKDIR) -p $(OUTDIR) ; fi
$(TARGET) : $(OUTDIR) $(COBJ) $(CPPOBJ)
	$(LINK)
include $(CDEF)
include $(CPPDEF)

PHONY = rebuild clean cleanall install
.PHONY : $(PHONY)
# Rebuild this project
rebuild : cleanall all
#
# Clean this project
clean :
	-$(RM) -f $(COBJ) $(CPPOBJ)
	-$(RM) -f $(TARGET)

# Clean this project and all dependencies
cleanall : clean
	-$(RM) $(CDEF) $(CPPDEF)

# Install lib or share
install:
	-install -s $(TARGET) -p -D $(USR_BIN_PATH)/$(TARGET)
uninstall:
	-$(RM) -f $(USR_BIN_PATH)/$(TARGET)
