#/*************************************************************        
#FileName : makefile   
#FileFunc : Linux编译链接源程序,生成目标文件,库或者可执行代码      
#Version  : V0.1        
#Author   : Sunrier        
#Date     : 2012-06-14   
#Descp    : Linux下makefile模板       
#*************************************************************/     
# Make command to use for dependencies
MAKE=make
RM=rm
MKDIR=mkdir
CC = gcc

# exec namae
ifndef NAME
NAME=$(notdir $(shell pwd))
endif

# target
TARGET=$(NAME)

# include
INC_PATH=.
INC=$(addprefix -I ,$(INC_PATH))

# search the lib which complied by myself
CFG_LIB_PATH=. 
CFG_LIB=$(wildcard $(addsuffix /*.a,$(CFG_LIB_PATH)))
CFG_LIB+=$(wildcard $(addsuffix /*.so,$(CFG_LIB_PATH)))
# other librarys
OTH_LIB=-lpthread
# Library of compling
LIB_PATH=$(addprefix -L ,$(CFG_LIB_PATH))
LIB:=$(LIBPATH) $(CFG_LIB) $(OTH_LIB)

# source of .c and .o
SRC_PATH=.
SRC=$(wildcard $(addsuffix /*.c,$(SRC_PATH)))
CFG_OBJ=$(patsubst %.c,%.o,$(notdir $(SRC)))
OBJS=$(addprefix $(OUTDIR)/,$(CFG_OBJ))
OBJ=$(SRC:.c=.o)

# dependent files .d
CDEF = $(SRC:.c=.d)

# Warning
WARNING=-Wall#-Wno-unused -Wno-format
CFLAGS+=$(WARNING) -O2 -Werror

#Configuration: Debug or Release
ifndef CFG
CFG=debug
endif

OUTDIR=
ifeq "$(CFG)" "debug"
OUTDIR=
#TARGET=debug/$(NAME)
CFLAGS+=-g
else
OUTDIR=
#TARGET=debug/lib$(NAME).so
endif

# complie
COMPILE=$(CC) $(CFLAGS) $(INC) $(LIB) -c $< -o $@ #$(OUTDIR)/$(*F).o

ifndef mode
mode=reg
LINK=$(CC) $(CFLAGS) -o $@  $(LIB) $(OBJ) #$(OUTDIR)/*.o
endif

ifeq "$(mode)" "lib"
LINK=ar -rs $@ $(OBJ)
TARGET=lib$(NAME).a
INSTALL_PATH=~/tools/library/lib/
endif

ifeq "$(mode)" "share"
LINK=$(CC) -shared -fpic $(CFLAGS) -o $@ $(OBJ)
TARGET=lib$(NAME).so
INSTALL_PATH=~/tools/library/share
endif

all:$(TARGET)

$(CDEF) : %.d : %.c
	$(MAKEDEPEND) $(<:.c=.o) $< > $@
depend :
	-rm -f $(CDEF)
	$(MAKE) $(CDEF)
$(OBJ) : %.o : %.c
	$(COMPILE)
$(OUTDIR) :  
	if test -n "$(OUTDIR)" ; then $(MKDIR) -p $(OUTDIR) ; fi
$(TARGET) : $(OUTDIR) $(OBJ)
	$(LINK)

include $(CDEF)
include $(CPPDEF)

PHONY=rebuild clean cleanall install
.PHONY : $(PHONY)
# Rebuild this project
rebuild : cleanall all
#
# Clean this project
clean :
	-$(RM) -f $(OBJ)
	-$(RM) -f $(TARGET)

# Clean this project and all dependencies
cleanall : clean
	#-$(RM) cscope.*
	#-$(RM) tags

# Install lib or share
install:
	find . -iname lib$(NAME).a -o -iname lib$(NAME).so | xargs -I{} cp {} ~/tools/lib/
	find . -iname "*.h" | xargs -I{} cp {} ~/tools/include
