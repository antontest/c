# appoint shell
SHELL=/bin/bash

# directory
DIRS=libxls
CURR_DIR=$(shell pwd)

.PHONY: all install clean cleanall rebuild $(DIRS)
# target
all: ${DIRS}
	@echo "cd ${DIRS}"
	-$(shell test ! -f ../../libs/libxl.so && ln -sf ${CURR_DIR}/lib/libxl.so ../../libs/)
	-$(shell test ! -f ../../incs/libxl/ && ln -sf ${CURR_DIR}/include_c/  ../../incs/libxl)

# target
install clean cleanall rebuild: $(DIRS)
	@echo -e "\e[1;35m opensource: make \e[1;36m$(MAKECMDGOALS) $@\e[1;0m"
	-rm -f ../../libs/libxl.so
	-rf -f ../../incs/libxl
