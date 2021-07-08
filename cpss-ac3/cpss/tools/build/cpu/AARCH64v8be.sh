# AARCH64v8 based CPUs, Big endian
# Target CPU settings

. $tool_build/cpu/AARCH64v8.sh

export ENDIAN=BE
export CPU_DIR=AARCH64v8be
export CROSS_COMPILE=aarch64_be-marvell-linux-gnu-


#WA
DEFAULT_LINUX_LSP_NAME=4.4/kernel
DEFAULT_LINUX_KERNEL_VERSION=4.4.52
DEFAULT_TOOLKIT_Linux=marvell_v8_64_be
CPU_PRE_TOOLKIT_marvell_v8_64_be=cpu_pre_toolkit_marvell_v8_64_be
DEFAULT_LINUX_CONFIG=mvebu_v8_be_lsp_defconfig
UNI_DIR=uni

# WA
cpu_pre_toolkit_marvell_v8_64_be()
{
    LINUX_CRFS_SCRIPT_NAME=crfs_arm_mv8_64_be.sh
	# WA: create defconfig for BE
	local c=$LSP_USER_BASE/uni/4.4/kernel/arch/arm64/configs/
	if [ -d $c ]; then
		if [ ! -f $c/$DEFAULT_LINUX_CONFIG ]; then
			cp $c/mvebu_v8_lsp_defconfig $c/$DEFAULT_LINUX_CONFIG
			echo "CONFIG_ARCH_SUPPORTS_BIG_ENDIAN=y" >> $c/$DEFAULT_LINUX_CONFIG
			echo "CONFIG_CPU_BIG_ENDIAN=y" >> $c/$DEFAULT_LINUX_CONFIG
			echo "CONFIG_CPU_ENDIAN_BE8=y" >> $c/$DEFAULT_LINUX_CONFIG
		fi
	fi
}
