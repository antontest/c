# appoint shell
SHELL=/bin/bash

# directory
DIRS= linked_list
DIRS+= get_args
DIRS+= enum

CUR_DIR=$(shell pwd)
LN_FROM_PATH=$(CUR_DIR)/utils.h
LN_TO_PATH=../../incs/utils.h
$(shell test ! -f $(LN_TO_PATH) && ln -s $(LN_FROM_PATH) $(LN_TO_PATH))
# target
all install clean cleanall rebuild: $(DIRS)
$(DIRS):
	@echo -e "\e[1;35m library: make \e[1;36m$(MAKECMDGOALS) $@\e[1;0m"
	$(MAKE) -C $@ $(MAKECMDGOALS)
clean:
	-rm ../../incs/utils.h
cleanall: clean

.PHONY: all install clean cleanall rebuild $(DIRS)
