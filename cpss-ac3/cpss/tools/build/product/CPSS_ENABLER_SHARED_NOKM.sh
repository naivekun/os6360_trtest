#

. $tool_build/product/CPSS_ENABLER_SHARED.sh
export LINUX_NOKM="YES"
export NO_KERN_PP_DRV=YES
PROJECT_DEFS="$PROJECT_DEFS LINUX_NOKM"
export LINUX_CONFIG_NOPRESTERA=YES
