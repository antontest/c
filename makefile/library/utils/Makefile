# appoint shell
SHELL=/bin/bash

# directory
DIRS= utils
DIRS+= linked_list
DIRS+= get_args
DIRS+= enum

# target
all install clean cleanall rebuild: $(DIRS)
$(DIRS):
	@echo -e "\e[1;35m library: make \e[1;36m$(MAKECMDGOALS) $@\e[1;0m"
	$(MAKE) -C $@ $(MAKECMDGOALS)
clean:
	-rm ../../incs/utils.h
cleanall: clean

.PHONY: all install clean cleanall rebuild $(DIRS)
