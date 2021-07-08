# Build Linux kernel only

DONT_CREATE_ROOTFS=YES
DONT_BUILD_IMAGES=YES

product_pre_build()
{
	return 0
}

os_build()
{
	return 0
}

os_link()
{
	return 0
}
