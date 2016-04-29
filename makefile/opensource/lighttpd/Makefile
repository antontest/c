# appoint shell
SHELL=/bin/bash

# directory
DIRS=lighttpd-1.4.39
CURR_DIR=$(shell pwd)

# compile 
yacc=$(shell type yacc)
lex=$(shell type lex)

# install 
install_path=$(HOME)/www
target_path=$(shell pwd)/${DIRS}

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
	-install -p -D -m 0555 ${target_path}/src/lighttpd $(install_path)/lighttpd
	-install -p -D -m 0444 ${target_path}/doc/lighttpd.conf $(install_path)/lighttpd.conf
	-install -p -D -m 0444 ${target_path}/doc/modules.conf $(install_path)/modules.conf
	-install -d ${target_path}/doc/config/conf.d $(install_path)/conf.d && install -p -D -m 0666 ${target_path}/doc/config/conf.d/* $(install_path)/conf.d/
	-install -d ${target_path}/src/.libs $(install_path)/modules && install -p -D -m 0555 ${target_path}/src/.libs/* $(install_path)/modules/
	
