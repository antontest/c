# appoint shell
SHELL=/bin/bash

# directory
DIRS=libxls
# DIRS+=xlslib
DIRS+=libxl
# DIRS+=boa
DIRS+=lighttpd
# DIRS+=mini_httpd
DIRS+=libsqlite3
DIRS+=libxlsxwriter

# target
all install clean cleanall rebuild: $(DIRS)
$(DIRS):
	@echo -e "\e[1;35m opensource: make \e[1;36m$(MAKECMDGOALS) $@\e[1;0m"
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: all install clean cleanall rebuild $(DIRS)
