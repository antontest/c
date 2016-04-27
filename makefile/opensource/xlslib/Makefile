# appoint shell
SHELL=/bin/bash

# directory
DIRS=xlslib
CURR_DIR=$(shell pwd)

.PHONY: all install clean cleanall rebuild $(DIRS)
# target
all: ${DIRS}
	@echo "cd ${DIRS}"
	-cd ${DIRS} && test ! -f Makefile && ./bootstrap && ./configure
	-cd ${DIRS} && make
	-$(shell test ! -f ../../libs/libxls.a && ln -sf ${CURR_DIR}/xlslib/src/.libs/libxls.a ../../libs/)
	-$(shell test ! -f ../../incs/xlslib && ln -sf ${CURR_DIR}/xlslib/src ../../incs/xlslib)

# target
clean cleanall: $(DIRS)
	@echo -e "\e[1;35m opensource: make \e[1;36m$(MAKECMDGOALS) \e[1;0m"
	-cd ${DIRS} && $(MAKE) $@ 
	-rm -rf ../../incs/xlslib
	-rm -rf ../../libs/libxls.a
