# appoint shell
SHELL=/bin/bash

# directory
DIRS=xlslib
CURR_DIR=$(shell pwd)

# target
all: ${DIRS}
	@echo "cd ${DIRS}"
	-cd ${DIRS} && ./bootstrap && ./configure 
	-make
	-test ! -f ../../libs/libxls.a && bln -sf ${CURR_DIR}/xlslib/src/.libs/libxls.a ../../libs/
	-test ! -f ../../incs/xlslib && ln -sf ${CURR_DIR}/xlslib/src ../../incs/xlslib

# target
clean cleanall: $(DIRS)
$(DIRS):
	@echo -e "\e[1;35m opensource: make \e[1;36m$(MAKECMDGOALS) $@\e[1;0m"
	$(MAKE) -C $@ $(MAKECMDGOALS)
	-rm -f ../../incs/xlslib
	-rm -f ../../libs/libxls.a

.PHONY: all install clean cleanall rebuild $(DIRS)
