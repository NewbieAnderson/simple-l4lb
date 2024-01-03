obj-m += src/lb_main.o
TARGET_MODULE = lb_main.ko
INSTALL_DIR = ./bin

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	mkdir -p $(INSTALL_DIR)
	cp src/$(TARGET_MODULE) $(INSTALL_DIR)/

clean:
	find . -name '*.o' -type f -delete
	find . -name '*.mod.c' -type f -delete
	find . -name '*.ko' -type f -delete
	find . -name 'Module.symvers' -type f -delete
	find . -name 'modules.order' -type f -delete
	find . -name '.modules.order.cmd' -type f -delete
	find . -name '*.mod' -type f -delete
	find . -name '*.cmd' -type f -delete
	find . -name '*.o.cmd' -type f -delete
	find . -name '*.mod.o.cmd' -type f -delete
	find . -name '*.mod.cmd' -type f -delete
	find . -name '*.ko.cmd' -type f -delete
	rm -rf $(INSTALL_DIR)