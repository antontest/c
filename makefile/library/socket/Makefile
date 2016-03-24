# appoint shell
SHELL=/bin/bash

# directory
DIRS=host 
DIRS+=property
DIRS+=tcp 
DIRS+=udp 
DIRS+=base
DIRS+=event
DIRS+=socket
DIRS+=cast
DIRS+=ftp

# target
all install clean cleanall rebuild: $(DIRS)
$(DIRS):
	@echo -e "\e[1;35m library socket: make \e[1;36m$(MAKECMDGOALS) $@\e[1;0m"
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: all install clean cleanall rebuild $(DIRS)
