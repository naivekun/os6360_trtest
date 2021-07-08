# Toolkit definitions for Yocto
# https://www.yoctoproject.org/
#
# This toolkit used to build application for custom target
# run on the same host
#

export TOOL=gnu
export TOOL_FAMILY=gnu

export TOOLKIT_MAKE_RULES=yocto

if [ "x$YOCTO_ENV" = "x" ]; then
	echo "YOCTO toolkit relies on some env variables to be set by environment setup script"
	echo "For example: /opt/yokto/1.8/environment-setup-i586-poky-linux"
	echo "Please assign YOCTO_ENV to the script path."
	echo "If not required, please assign any non empty value (export YOCTO_ENV=1)."
	exit 1
fi

# import YOKTO environment
. $YOCTO_ENV

export GCC_VER=`$CC -v 2>&1 | awk '/gcc version/ {print $3}'`

