# ARMv7 CPU with Linux kernel 3.10
# Target CPU settings

. $tool_build/cpu/ARMv7.sh

DEFAULT_LINUX_LSP_NAME=3.10/kernel
DEFAULT_LINUX_KERNEL_VERSION=3.10.70
DEFAULT_LINUX_CONFIG=mvebu_lsp_defconfig
LSP_KERNEL_TYPE=zImage
LSP_KERNEL_CONFIG=$DEFAULT_LINUX_CONFIG
export UNI_DIR=uni

LSP_KERNEL_DTB_NAME[0]=msys-bobk-caelum-db.dtb
LSP_KERNEL_UDTB_NAME[0]=msys_caelum

LSP_KERNEL_DTB_NAME[1]=msys-bobk-cetus-db.dtb
LSP_KERNEL_UDTB_NAME[1]=msys_cetus

LSP_KERNEL_DTB_NAME[2]=msys-bc2-db.dtb
LSP_KERNEL_UDTB_NAME[2]=msys_bc2

LSP_KERNEL_DTB_NAME[3]=msys-bobk-cygnus-rd.dtb
LSP_KERNEL_UDTB_NAME[3]=msys_cygnus

LSP_KERNEL_DTB_NAME[4]=msys-bobk-lewis-rd.dtb
LSP_KERNEL_UDTB_NAME[4]=msys_lewis

LSP_KERNEL_DTB_NAME[5]=msys-ac3-db.dtb
LSP_KERNEL_UDTB_NAME[5]=msys_ac3

LSP_KERNEL_DTB_NAME[6]=armada-385-customer3.dtb
LSP_KERNEL_UDTB_NAME[6]=385

LSP_KERNEL_DTB_NAME[7]=armada-38x-interposer.dtb
LSP_KERNEL_UDTB_NAME[7]=ac3x

LSP_KERNEL_DTB_NAME[8]=armada-38x-interposer-all-pci.dtb
LSP_KERNEL_UDTB_NAME[8]=pipex4

info_message "DEFAULT_LINUX_LSP_NAME=${DEFAULT_LINUX_LSP_NAME}"
info_message "DEFAULT_LINUX_KERNEL_VERSION=${DEFAULT_LINUX_KERNEL_VERSION}"
info_message "DEFAULT_LINUX_CONFIG=${DEFAULT_LINUX_CONFIG}"

info_message "UNI_DIR=${UNI_DIR}"
