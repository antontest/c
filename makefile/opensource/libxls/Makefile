# appoint shell
SHELL=/bin/bash

# directory
DIRS=libxls
CURR_DIR=$(shell pwd)

.PHONY: all install clean cleanall rebuild $(DIRS)
# target
all: ${DIRS}
	@echo "cd ${DIRS}"
	-cd ${DIRS} && make
	-$(shell test ! -f ../../libs/libxlsreader.so && ln -sf ${CURR_DIR}/libxls/src/.libs/libxlsreader.so.1.4.0 ../../libs/libxlsreader.so)
	-$(shell test ! -f ../../incs/libxls/ && ln -sf ${CURR_DIR}/libxls/include/libxls ../../incs/)

# target
install clean cleanall rebuild: $(DIRS)
	@echo -e "\e[1;35m opensource: make \e[1;36m$(MAKECMDGOALS) $@\e[1;0m"
	-cd ${DIRS} && $(MAKE) $@ 
	-rm -f ../../incs/libxls 
	-rm -f ../../libs/libxlsreader.so
