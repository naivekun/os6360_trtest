# ARMv7 CPU with Linux kernel 4.4
# Target CPU settings

. $tool_build/cpu/ARMv7.sh

DEFAULT_LINUX_LSP_NAME=4.4/kernel
DEFAULT_LINUX_KERNEL_VERSION=4.4.52
DEFAULT_LINUX_CONFIG=mvebu_v7_lsp_defconfig
LSP_KERNEL_TYPE=zImage
LSP_KERNEL_CONFIG=$DEFAULT_LINUX_CONFIG
export UNI_DIR=uni

LSP_KERNEL_DTB_NAME[0]=msys-ac3-db.dtb
LSP_KERNEL_UDTB_NAME[0]=msys_ac3

LSP_KERNEL_DTB_NAME[1]=armada-385-amc.dtb
LSP_KERNEL_UDTB_NAME[1]=385

LSP_KERNEL_DTB_NAME[2]=armada-38x-interposer.dtb
LSP_KERNEL_UDTB_NAME[2]=ac3x

LSP_KERNEL_DTB_NAME[3]=msys-ac3-rd.dtb
LSP_KERNEL_UDTB_NAME[3]=msys_ac3_rd

echo "DEFAULT_LINUX_LSP_NAME=${DEFAULT_LINUX_LSP_NAME}"
echo "DEFAULT_LINUX_KERNEL_VERSION=${DEFAULT_LINUX_KERNEL_VERSION}"
echo "DEFAULT_LINUX_CONFIG=${DEFAULT_LINUX_CONFIG}"

echo "UNI_DIR=${UNI_DIR}"
