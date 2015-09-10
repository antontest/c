#/*************************************************************        
#FileName : makefile   
#FileFunc : Linux编译链接源程序,生成目标文件,库或者可执行代码      
#Version  : V0.1        
#Author   : Sunrier        
#Date     : 2012-06-14   
#Descp    : Linux下makefile模板       
#*************************************************************/     
# Make command to use for dependencies
MAKE = make
RM = rm
MKDIR = mkdir
CC = gcc
XX = g++

# target
TARGET= fib

# include
INCLUDE_PATH = .
INC = $(addprefix -I ,$(INCLUDE_PATH))

# search the lib which complied by myself
CFG_LIB_PATH =
# other librarys
OTH_LIB =

# source of .c and .o
SRC_PATH = .
CSRC = $(wildcard $(addsuffix /*.c,$(SRC_PATH)))
CPPSRC = $(wildcard $(addsuffix /*.cpp,$(SRC_PATH)))
COBJ = $(CSRC:.c=.o)
CPPOBJ = $(CPPSRC:.cpp=.o)

# dependent files .d
CDEF = $(CSRC:.c=.d)
CPPDEF = $(CPPSRC:.cpp=.d)

# top dir
TOPDIR:=$(shell \
	basename $$(dirname $$(pwd)) \
)
ifeq "$(TOPDIR)" "lib"
mode=lib
endif

# Warning
WARNING = -Wall#-Wno-unused -Wno-format
CFLAGS += $(WARNING) -O2 -Werror

#Configuration: Debug or Release
ifndef CFG
CFG = debug
endif

OUTDIR =
ifeq "$(CFG)" "debug"
OUTDIR =
CFLAGS += -g
else
OUTDIR =
endif

ifneq "$(CPPOBJ)" ""
CFLAGS += -lstdc++
endif

# complie
COMPILE = $(CFLAGS) $(INC) -c $< -o $@  $(LIB) #$(OUTDIR)/$(*F).o

# exec namae
ifndef NAME
NAME =
endif

ifeq "$(NAME)" ""
NAME = $(notdir $(shell pwd))
endif

ifneq "$(TARGET)" ""
NAME := $(TARGET)
endif

## complie mode setting
ifndef mode
mode = reg
TARGET = $(NAME)
CFG_LIB_PATH += .
LINK = $(CC) $(CFLAGS) -o $@ $(COBJ) $(CPPOBJ) $(LIB) #$(OUTDIR)/*.o
endif
# compile library
ifeq "$(mode)" "lib"
LINK = ar -rs $@ $(COBJ) $(CPPOBJ)
TARGET = lib$(NAME).a
endif

#compile share
ifeq "$(mode)" "share"
LINK=$(CC) -shared -fpic $(CFLAGS) -o $@ $(COBJ) $(CPPOBJ) $(LIB)
TARGET = lib$(NAME).so
endif

# Library of compling
LIB_PATH = $(addprefix -L ,$(CFG_LIB_PATH))
# set lib
CFG_LIB = $(wildcard $(addsuffix /*.a,$(CFG_LIB_PATH)))
CFG_LIB += $(wildcard $(addsuffix /*.so,$(CFG_LIB_PATH)))
LIB := $(LIB_PATH) $(CFG_LIB) $(OTH_LIB)

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
	-mktg
	-mkcs
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
	-$(RM) cscope.*
	-$(RM) tags
	-$(RM) $(CDEF) $(CPPDEF)

# Install lib or share
install:
	find . -iname lib$(NAME).a -o -iname lib$(NAME).so | xargs -I{} cp {} ~/tools/lib/
	find . -iname "*.h" | xargs -I{} cp {} ~/tools/include
