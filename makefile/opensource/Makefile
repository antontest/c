# appoint shell
SHELL=/bin/bash

# directory
DIRS=libxls
DIRS+=libxl
DIRS+=lighttpd
DIRS+=libsqlite3
DIRS+=libxml2
# DIRS+=xlslib
# DIRS+=boa
# DIRS+=mini_httpd
# DIRS+=libxlsxwriter

# target
all install clean cleanall rebuild: $(DIRS)
$(DIRS):
	@echo -e "\e[1;35m opensource: make \e[1;36m$(MAKECMDGOALS) $@\e[1;0m"
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: all install clean cleanall rebuild $(DIRS)
