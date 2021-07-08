# CPSS enabler without kernel module

. $tool_build/product/CPSS_ENABLER.sh
export LINUX_NOKM="YES"
export NO_KERN_PP_DRV=YES
PROJECT_DEFS="$PROJECT_DEFS LINUX_NOKM"
export LINUX_CONFIG_NOPRESTERA=YES
