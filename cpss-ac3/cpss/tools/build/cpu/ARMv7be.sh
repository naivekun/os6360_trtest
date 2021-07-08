# ARMv7 based CPUs, Big endian
# Target CPU settings

. $tool_build/cpu/ARMv7.sh

export ENDIAN=BE
export CPU_DIR=ARMv7be
export ARM_CPU=ARMv7be

DEFAULT_TOOLKIT_Linux=marvell_v7sft_be
CPU_PRE_TOOLKIT_marvell_v7sft_be=cpu_pre_toolkit_marvell_v7sft_be

cpu_pre_toolkit_marvell_v7sft_be()
{
    LINUX_CRFS_SCRIPT_NAME=crfs_arm_mv7sft.sh
}
