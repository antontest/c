# appoint shell
SHELL=/bin/bash

# directory
DIRS=mutex 
DIRS+=cond 
DIRS+=thread_value
DIRS+=bsem
DIRS+=thread
DIRS+=rwlock
DIRS+=pool

# target
all install clean cleanall rebuild: $(DIRS)
$(DIRS):
	@echo -e "\e[1;35m library thread: make \e[1;36m$(MAKECMDGOALS) $@\e[1;0m"
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: all install clean cleanall rebuild $(DIRS)
