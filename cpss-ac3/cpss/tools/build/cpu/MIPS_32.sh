#
# MIPS32 CPU support
# 32-bit kernel
# 32-bit applications
#
# File version: 1

#export CPU=MIPS64
export CPU=MIPS32
export CPU_TOOL=mips
export CPU_DIR=MIPS32

export LIB_1=32

export CPU_FAMILY=MIPS_CPU
#export WORDSIZE_REQUIRED="64"
export WORDSIZE_REQUIRED="32"
export ENDIAN=BE

CPU_FLAGS="-mlong-calls -mgp32 -mfp32 "
DEFAULT_TOOLKIT_Linux=mips32
#CPU_PRE_TOOLKIT_mips64=cpu_pre_toolkit_mips64
CPU_PRE_TOOLKIT_mips32=cpu_pre_toolkit_mips32
DEFAULT_MANAGEMENT_BUS=PCI_SMI
#PROJECT_DEFS="$PROJECT_DEFS PHYSADDR_IS_64BIT KERNEL_IS_64BIT"
PROJECT_DEFS="$PROJECT_DEFS PHYSADDR_IS_32BIT KERNEL_IS_32BIT"


if [ $TARGET_OS = Linux ]
then
  #DEFAULT_LINUX_LSP_NAME=mips64_lsp
  DEFAULT_LINUX_LSP_NAME=mips32_lsp
  DEFAULT_LINUX_KERNEL_VERSION=2.6.27.7
  #DEFAULT_LINUX_CONFIG=mv_mips64_defconfig
  DEFAULT_LINUX_CONFIG=mv_mips32_defconfig
  LINUX_CRFS_ARCH_ARG=""
  export MV_KERNEL_EXT_USE=${MV_KERNEL_EXT_USE:-yes}
  export MV_KERNEL_EXT_BUILD=${MV_KERNEL_EXT_BUILD:-yes}
fi


cpu_pre_toolkit_mips64()
{
  LINUX_CRFS_SCRIPT_NAME=crfs_mips64.sh
}

linux_2_6_cr_jffs2_params()
{
  UIMAGE_PATH="${DIST}"
	UIMAGE_NO_COPY=TRUE
  #CR_JFFS2_FILE_NAME="cr_jffs2_image_mips64.sh"
  CR_JFFS2_FILE_NAME="cr_jffs2_image_mips32.sh"
  CR_JFFS2_PARAMS="${IMAGE_PATH} ${DIST}/rootfs"
}
