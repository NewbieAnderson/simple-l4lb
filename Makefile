obj-m += src/lb_main.o
TARGET_MODULE = lb_main.ko
INSTALL_DIR = ./bin

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	mkdir -p $(INSTALL_DIR)
	cp src/$(TARGET_MODULE) $(INSTALL_DIR)/
	rm -f *.cmd
	rm -f *.swp
	rm -f *.symvers
	rm -f *.order
	rm -f *.Module*
	rm -f *.modules*
	rm -f *.Module.symvers.cmd
	rm -f *.modules.order.cmd
	rm -f *.Makefile.swp

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -rf $(INSTALL_DIR)
	rm -f *~
	rm -f *.o
	rm -f *.cmd
	rm -f *.mod.c *.symvers *.order
