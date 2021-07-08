# ARMv7 based CPUs
# Target CPU settings

DEFAULT_MANAGEMENT_BUS=PCI_SMI_I2C

export CPU=ARMARCH7
export CPU_TOOL=arm
export CPU_FAMILY=CPU_ARM
export CPU_DIR=ARMv7
export ARM_CPU=ARMv7

export linux_objects_ext=ARMv7
export ARCH=arm
export CROSS_COMPILE=arm-marvell-linux-gnueabi-

#
DEFAULT_TOOLKIT_Linux=marvell_v7sft
CPU_PRE_TOOLKIT_marvell_v7sft=cpu_pre_toolkit_marvell_v7sft

#
###############################
# Set CPSS LOG_ENABLE
###############################
if [ -z $API_LOG_ENABLE ]; then
    export LOG_ENABLE="L_ON"
fi

cpu_pre_toolkit_marvell_v7sft()
{
    LINUX_CRFS_SCRIPT_NAME=crfs_arm_mv7sft.sh
}
linux_2_6_cr_jffs2_params()
{
  UIMAGE_PATH="${DIST}"
  UIMAGE_NO_COPY=TRUE
  echo "echo hello" >$DIST/ubi_script.txt
  CR_JFFS2_FILE_NAME="cr_jffs2_img_arm_mv7sft.sh"
  CR_JFFS2_PARAMS="\
        ${DIST}/arch/${CPU_TOOL}/boot/zImage \
        ${DIST}/arch/${CPU_TOOL}/boot/dts/${1:-$LSP_KERNEL_DTB_NAME} \
        ${DIST}/rootfs \
        little_endian \
        ${IMAGE_PATH} \
        $DIST/ubi_script.txt"
}
