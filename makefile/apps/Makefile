# appoint shell
SHELL=/bin/bash

# directory
DIRS= 
#DIRS+= 

# target
all install clean cleanall rebuild: $(DIRS)
$(DIRS):
	@echo -e "\e[1;35m apps: make \e[1;36m$(MAKECMDGOALS) $@\e[1;0m"
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: all install clean cleanall rebuild $(DIRS)
