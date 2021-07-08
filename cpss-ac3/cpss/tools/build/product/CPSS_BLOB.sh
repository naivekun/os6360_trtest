# Link common/ mainPpDrv/ mainTmDrv into one ELF object
# This object can be linked with kernel module
# No external references are allowed in this object

export INCLUDE_UTF=NO
export INCLUDE_ENHANCED_UTF=NO
export IMPL_TGF=NO
. $tool_build/product/cpss_common.inc

unset LOG_ENABLE
export API_LOG_ENABLE="NO"

export MAKE_TARGET_LINK=cpss_blob

export CPSS_LIB_ONLY=YES
export LINUX_CPSS_TARGET=NO_CPSS_ENABLER
PROJECT_DEFS="$PROJECT_DEFS CPSS_BLOB NO_KERN_PP_DRV"
export CPSS_BLOB=YES
export NO_KERN_PP_DRV=YES
export INCLUDE_UTF=NO
export INCLUDE_ENHANCED_UTF=NO
export CMD_LUA_CLI=no
export EXCLUDE_GALTIS=EXCLUDE_LIB

# owerride to finish at this stage
product_post_build()
{
	trace_message "=> END build blob"
	exit 0
}

###DONT_LINK=YES
export LINUX_BUILD_KERNEL=NO
