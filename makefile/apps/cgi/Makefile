# appoint shell
SHELL=/bin/bash

# directory
DIRS= setup
DIRS+= upload
DIRS+= login
DIRS+= user_admin
DIRS+= user_add
DIRS+= user_detail
DIRS+= user_delete
DIRS+= user_update
DIRS+= proc_show

# target
all install uninstall clean cleanall rebuild: $(DIRS)
$(DIRS):
	@echo -e "\e[1;35m cgi: make \e[1;36m$(MAKECMDGOALS) $@\e[1;0m"
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: all install clean cleanall rebuild $(DIRS)
