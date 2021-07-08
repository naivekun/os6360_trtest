# Toolkit definitions for
# freescale - freescale-2011.03
#
#


COMP_PREFIX=powerpc-linux-gnu-
export CROSS_COMPILE=$COMP_PREFIX

export TOOL=gnu
export TOOL_FAMILY=gnu
export TOOLKIT_MAKE_RULES=freescale

export COMP_PREFIX
export PREFIX=$COMP_PREFIX
export POSTFIX=$COMP_POSTFIX
export G=g
COMPILER=${PREFIX}${G}cc${POSTFIX}
export AS=${PREFIX}as${POSTFIX}
export CC=${PREFIX}${G}cc${POSTFIX}
export AR=${PREFIX}ar${POSTFIX}
export LD=${PREFIX}ld${POSTFIX}
export NM=${PREFIX}nm${POSTFIX}
export CPP=${PREFIX}cpp${POSTFIX}
export STRIP=${PREFIX}strip${POSTFIX}
export OBJCOPY=${PREFIX}objcopy${POSTFIX}
export GNU_CROSS=yes

export GCC_VER=`$COMPILER -v 2>&1 | awk '/gcc version/ {print $3}'`

TOOLKIT_PATH=`which $COMPILER`
if [ $? != 0 ]; then
	echo "**********************************************"
	echo "**********************************************"
	echo "!!! $COMPILER not found in \$PATH"
	echo "!!! Make sure you have set correct PATH value"
	exit 1
fi
TOOLKIT_PATH=`dirname $TOOLKIT_PATH`
TOOLKIT_PATH=`dirname $TOOLKIT_PATH`
export COMPILER_ROOT=$TOOLKIT_PATH
export XCOMP_ROOT_PATH_SUFFIX=powerpc-linux-gnu/libc
export XCOMP_ROOT_PATH=$COMPILER_ROOT/$XCOMP_ROOT_PATH_SUFFIX
