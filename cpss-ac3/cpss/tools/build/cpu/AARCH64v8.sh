# AARCH64v8 based CPUs
# Target CPU settings

DEFAULT_MANAGEMENT_BUS=PCI_SMI_I2C

export CPU=AARCH64v8
export CPU_TOOL=arm64
export CPU_FAMILY=CPU_ARM64
export CPU_DIR=AARCH64v8

export ARCH=arm64
export CROSS_COMPILE=aarch64-marvell-linux-gnu-

#
DEFAULT_TOOLKIT_Linux=marvell_v8_64
CPU_PRE_TOOLKIT_marvell_v8_64=cpu_pre_toolkit_marvell_v8_64

DEFAULT_LINUX_LSP_NAME=4.4/kernel
DEFAULT_LINUX_KERNEL_VERSION=4.4.52
DEFAULT_LINUX_CONFIG=mvebu_v8_lsp_defconfig
LSP_KERNEL_TYPE=Image
LSP_KERNEL_CONFIG=$DEFAULT_LINUX_CONFIG
export UNI_DIR=uni

LSP_KERNEL_DTB_NAME[0]=marvell/armada-7020-amc.dtb

#
###############################
# Set CPSS LOG_ENABLE
###############################
if [ -z $API_LOG_ENABLE ]; then
    export LOG_ENABLE="L_ON"
fi

cpu_pre_toolkit_marvell_v8_64()
{
    LINUX_CRFS_SCRIPT_NAME=crfs_arm_mv8_64.sh
}
linux_2_6_cr_jffs2_params()
{
  UIMAGE_PATH="${DIST}"
  UIMAGE_NO_COPY=TRUE
  echo "echo hello" >$DIST/ubi_script.txt
  CR_JFFS2_FILE_NAME="cr_img_arm_mv8_64.sh"
  CR_JFFS2_PARAMS="\
        ${DIST}/arch/${CPU_TOOL}/boot/zImage \
        ${DIST}/arch/${CPU_TOOL}/boot/dts/${1:-$LSP_KERNEL_DTB_NAME} \
        ${DIST}/rootfs \
        little_endian \
        ${IMAGE_PATH} \
        $DIST/ubi_script.txt"
}
