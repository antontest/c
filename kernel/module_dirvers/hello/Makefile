CONFIG_MODULE_SIG=n
CONFIG_MODULE_SIG_ALL=n
obj-m:=hello.o
hellomodule-objs:=module
KDIR:=/lib/modules/$(shell uname -r)/build
MAKE:=make 
default:  
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules  
clean:  
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean  

