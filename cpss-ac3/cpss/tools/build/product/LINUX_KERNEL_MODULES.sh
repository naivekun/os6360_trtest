# Build Linux kernel modules only
# Linux kernel will also be built

DONT_CREATE_ROOTFS=YES

export USER_BASE=$CPSS_PATH
export USER_MAKE=presteraTopMake

export BUILD_ONLY="cpssEnabler/mainExtDrv/src/gtExtDrv cpssEnabler/mainOs/src/gtOs/linux/kernelExt"
FAST_COMPILE=NO
DONT_BUILD_IMAGES=YES
MAKE_TARGET=modules

product_pre_build()
{
	test -d $DIST || return 1
	mkdir -p $DIST/rootfs/lib/modules
}

os_link()
{
	return 0
}
