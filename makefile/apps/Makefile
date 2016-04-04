# appoint shell
SHELL=/bin/bash

# directory
DIRS=tcp
DIRS+=udp 
DIRS+=cmd_agent
DIRS+=task_manager
DIRS+=socket
DIRS+=arp
DIRS+=ftp

# target
all install clean cleanall rebuild: $(DIRS)
$(DIRS):
	@echo -e "\e[1;35m apps: make \e[1;36m$(MAKECMDGOALS) $@\e[1;0m"
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: all install clean cleanall rebuild $(DIRS)
