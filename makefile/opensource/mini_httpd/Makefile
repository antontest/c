# appoint shell
SHELL=/bin/bash

# directory
DIRS=mini_httpd-1.23
CURR_DIR=$(shell pwd)

# compile 
yacc=$(shell type yacc)
lex=$(shell type lex)

# install 
install_path=$(USR_TOOLS_PATH)/web
target_path=$(shell pwd)

.PHONY: all install clean cleanall rebuild $(DIRS)
# target
all: ${DIRS}
	@echo "cd ${DIRS}"
	-test ! -f ${DIRS}/Makefile && cd ${DIRS} && ./configure
	-cd ${DIRS} && make

# target
clean cleanall: 
	@echo -e "\e[1;35m opensource: make \e[1;36m$(MAKECMDGOALS) $@\e[1;0m"
	-cd ${DIRS} && make clean
rebuild: clean all
	@echo -e "\e[1;35m opensource: make \e[1;36m$(MAKECMDGOALS) $@\e[1;0m"
install:
	-install -p -D -m 0555 ${target_path}/mini_httpd $(install_path)/mini_httpd
	-install -p -D -m 0444 ${target_path}/conf/mini.conf $(install_path)/mini.conf
	
