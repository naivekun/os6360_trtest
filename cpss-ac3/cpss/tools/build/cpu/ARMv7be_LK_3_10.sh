# ARMv7 CPU with Linux kernel 3.10 - Big endian
# Target CPU settings

. $tool_build/cpu/ARMv7be.sh

DEFAULT_LINUX_LSP_NAME=3.10/kernel
DEFAULT_LINUX_KERNEL_VERSION=3.10.70
DEFAULT_LINUX_CONFIG=mvebu_be_lsp_defconfig
LSP_KERNEL_TYPE=zImage
LSP_KERNEL_CONFIG=$DEFAULT_LINUX_CONFIG
export UNI_DIR=uni

LSP_KERNEL_DTB_NAME[0]=armada-385-customer3.dtb
LSP_KERNEL_UDTB_NAME[0]=385

# WA
cpu_pre_toolkit_marvell_v7sft_be()
{
    LINUX_CRFS_SCRIPT_NAME=crfs_arm_mv7sft.sh
	# WA: create defconfig for BE
	local c=$LSP_USER_BASE/uni/3.10/kernel/arch/arm/configs
	if [ -d $c ]; then
		if [ ! -f $c/mvebu_be_lsp_defconfig ]; then
			cp $c/mvebu_lsp_defconfig $c/mvebu_be_lsp_defconfig
			echo "CONFIG_ARCH_SUPPORTS_BIG_ENDIAN=y" >> $c/mvebu_be_lsp_defconfig
			echo "CONFIG_CPU_BIG_ENDIAN=y" >> $c/mvebu_be_lsp_defconfig
			echo "CONFIG_CPU_ENDIAN_BE8=y" >> $c/mvebu_be_lsp_defconfig
		fi
	fi
}
