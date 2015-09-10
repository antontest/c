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

# exec namae
ifndef NAME
NAME=$(notdir $(shell pwd))
endif

#
## Configuration:target src obj lib flags oomplil
#
# Warning
WARNING=-Wall  #-Wno-unused -Wno-format
COMM_FLAGS=-O2 -Werror

# source of .c and .o
SRC_PATH=. src
CFG_SRC=$(wildcard $(addsuffix /*.c,$(SRC_PATH)))
CFG_OBJ=$(patsubst %.c,%.o,$(OUTDIR)/$(notdir $(CFG_SRC)))
COMMON_OBJ=
OBJ=$(COMMON_OBJ) $(CFG_OBJ)
#
# include
INC_PATH=. inc
CFG_INC=$(addprefix -I./,$(INC_PATH))
#
# lib
LIB_PATH=. 
CFG_LIB=$(wildcard $(addsuffix /*.lib,$(LIB_PATH)))
#
# complie
COMPILE=gcc $(WARNING) $(COMM_FLAGS) -c -o "$(OUTDIR)/$(*F).o" $(CFG_INC) "$<"

# If no configuration is specified, "Debug" will be used
ifndef CFG
CFG=debug
endif

#
## Configuration: Debug or Release
#
ifeq "$(CFG)" "debug"
OUTDIR=debug
CFG_FLAGS=-g $(WARNING) $(COMM_FLAGS)
else
OUTDIR=release
CFG_FLAGS=$(WARNING) $(COMM_FLAGS)
endif

#
## Configuration: Mode
#
ifndef MOD
MOD=reg
endif

ifeq "$(MOD)" "reg"
OUTFILE=$(NAME).exe
LINK=gcc $(CFG_FLAGS) -o "$(OUTFILE)" $(OBJ) $(CFG_LIB)
endif

ifeq "$(MOD)" "lib"
OUTFILE=$(OUTDIR)/lib$(NAME).a
LINK=ar -rs "$(OUTFILE)" $(OBJ)
endif

ifeq "$(MOD)" "share"
OUTFILE=$(OUTDIR)/lib$(NAME).so
LINK=gcc -shared -fpic $(CFG_FLAGS) -o "$(OUTFILE)" $(OBJ) $(CFG_LIB)
endif

#Options -O for release and -g for development  
#-w:不生成任何警告信息  
#-Wall:输出所有的警告信息  
#-O:在编译时进行优化(注:O为大写字母)  
#-O0:不进行优化处理  
#-O或-O1:优化生成代码,缺省值  
#-O2:进一步优化  
#-O3:比-O2更进一步优化,包括inline函数  
#-g:表示编译debug版本,生成调试信息(注:使用GNU的gdb必须加入这个选项)  
#-ansr:只支持ANSI标准的C语法,这一选项将禁止GNU C的某些特色,例如asm或typeof关键词    
#-werror:把所有警告转换为错误,以在警告发生时中止编译过程       


# Pattern rules
$(OUTDIR)/%.o : $(CFG_SRC)
	$(COMPILE)

all: $(OUTFILE)
$(OUTFILE): $(OUTDIR) $(OBJ)
	$(LINK)

$(OUTDIR):
	$(MKDIR) -p "$(OUTDIR)"

.PHONY: $(PHONY)
# Rebuild this project
rebuild: cleanall all
#
# Clean this project
clean:
	$(RM) -f $(OUTFILE)
	$(RM) -f $(OBJ)

# Clean this project and all dependencies
cleanall: clean
