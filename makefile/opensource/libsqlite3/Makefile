# appoint shell
SHELL=/bin/bash

# directory
DIRS=sqlite3
CURR_DIR=$(shell pwd)

# install 
install_path=$(USR_LIB_PATH)
target_path=$(shell pwd)/${DIRS}

# header and target LINK 
CUR_DIR_PATH=$(shell pwd)
CUR_DIR=$(shell basename `pwd`)
LN_FROM_INC=$(CUR_DIR_PATH)/${DIRS}
LN_FROM_TARGET=$(CUR_DIR_PATH)/${DIRS}/.libs/libsqlite3.a
LN_TO_INC=../../incs/$(DIRS)
LN_TO_TARGET=../../libs/libsqlite3.a

.PHONY: all install clean cleanall rebuild $(DIRS)
# target
all: ${DIRS}
	@echo "cd ${DIRS}"
	-test ! -f ${DIRS}/Makefile && cd ${DIRS} && ./configure
	-cd ${DIRS} && make
	-$(shell test ! -L $(LN_TO_INC) && ln -sf $(LN_FROM_INC) $(LN_TO_INC))
	-$(shell test ! -L $(LN_TO_TARGET) && ln -sf $(LN_FROM_TARGET) $(LN_TO_TARGET))

# target
clean cleanall: 
	@echo -e "\e[1;35m opensource: make \e[1;36m$(MAKECMDGOALS) $@\e[1;0m"
	-cd ${DIRS} && make clean
	-rm -f $(LN_TO_INC)
	-rm -f $(LN_TO_TARGET)

rebuild: clean all
	@echo -e "\e[1;35m opensource: make \e[1;36m$(MAKECMDGOALS) $@\e[1;0m"

install:
	-install -p -D -m 0555 ${target_path}/src/lighttpd $(install_path)/lighttpd
	-install -p -D -m 0444 ${target_path}/doc/lighttpd.conf $(install_path)/lighttpd.conf
	-install -p -D -m 0444 ${target_path}/doc/modules.conf $(install_path)/modules.conf
	-install -d ${target_path}/doc/config/conf.d $(install_path)/conf.d && install -p -D -m 0666 ${target_path}/doc/config/conf.d/* $(install_path)/conf.d/
	-install -d ${target_path}/src/.libs $(install_path)/modules && install -p -D -m 0555 ${target_path}/src/.libs/* $(install_path)/modules/
	
