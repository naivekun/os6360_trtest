# -*-makefile-*-

# call from kernel build system
TARGETS = mvKernelExt

# flag kernel patches
ifeq (yes,$(KERNEL_SYSCALL_TABLE_EXPORTED))
	EXTRA_CFLAGS += -DMVKERNELEXT_SYSCALLS
endif
ifeq (yes,$(KERNEL_TASK_STRUCT_EXTENDED))
	EXTRA_CFLAGS += -DMVKERNELEXT_TASK_STRUCT
endif
ifeq (1, $(LINUX_SIM))
	EXTRA_CFLAGS += -DLINUX_SIM
endif
ifeq (arm, $(CPU_TOOL))
	EXTRA_CFLAGS += -DCPU_ARM
endif
ifneq (,$(filter 3.% 4.%,$(KERNELRELEASE)))
	MV_REALTIME_SCEDULLING_REQUIRED = yes
endif
ifneq (,$(MV_REALTIME_SCEDULLING_REQUIRED))
	EXTRA_CFLAGS += -DENABLE_REALTIME_SCHEDULING_POLICY
endif




ifneq ($(KERNELRELEASE),)

obj-m := $(addsuffix .o,$(TARGETS))
mvKernelExt-y := main.o mv_tasks.o mv_sem.o mv_msgq.o mv_util.o

else
#
# CPSS make

all: default

default:
	$(MAKE) -C $(DIST_HOME) M=$(shell pwd) modules
	@echo -e "\tstripping debug information from $(addsuffix .ko,$(TARGETS))"
	$(PREFIX)strip$(POSTFIX) -g $(addsuffix .ko,$(TARGETS))
	sh inst_mod.sh $(DIST_HOME)/rootfs 244 1

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions modules.order Module.symvers

depend .depend dep:
	$(CC) $(EXTRA_CFLAGS) -M *.c > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif

endif
