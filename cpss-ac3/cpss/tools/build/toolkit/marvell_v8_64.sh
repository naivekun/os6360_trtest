# toolkit defs

COMP_PREFIX=aarch64-marvell-linux-gnu-
XCOMP_ROOT_PATH_SUFFIX=aarch64-linux-gnu/libc

export TOOL=gnu
export TOOL_FAMILY=gnu
export TOOLKIT_MAKE_RULES=marvell_v8_64
export G=g
export COMP_PREFIX
export PREFIX=$COMP_PREFIX
export POSTFIX=$COMP_POSTFIX
COMPILER=${PREFIX}${G}cc${POSTFIX}
export AS=${PREFIX}as${POSTFIX}
export CC=${PREFIX}${G}cc${POSTFIX}
export AR=${PREFIX}ar${POSTFIX}
export LD=${PREFIX}ld${POSTFIX}
export NM=${PREFIX}nm${POSTFIX}
export CPP=${PREFIX}cpp${POSTFIX}
export GPP=${PREFIX}g++${POSTFIX}
export STRIP=${PREFIX}strip${POSTFIX}
export OBJCOPY=${PREFIX}objcopy${POSTFIX}
export GNU_CROSS=yes

export XCOMP_ROOT_PATH_SUFFIX

TOOLKIT_PATH=`which $COMPILER`
if [ $? != 0 ]; then
	# WA: add Marvell's default toolchain path
	PATH=$PATH:/swtools/devtools/gnueabi/arm_le/aarch64v8-marvell-linux-gnu/bin
	TOOLKIT_PATH=`which $COMPILER`
fi
if [ $? != 0 ]; then
	echo "**********************************************"
	echo "**********************************************"
	echo "!!! $COMPILER not found in \$PATH"
	echo "!!! Make sure you have set correct PATH value"
	exit 1
fi
TOOLKIT_PATH=`dirname $TOOLKIT_PATH`

export GCC_VER=`$COMPILER -v 2>&1 | awk '/gcc version/ {print $3}'`

export COMPILER_ROOT=$TOOLKIT_PATH
export TOOL_CHAIN_PATH=$TOOLKIT_PATH
TOOLKIT_PATH=`dirname $TOOLKIT_PATH`
export XCOMP_INC_PATH=$TOOLKIT_PATH

if [ ! -d $TOOLKIT_PATH/$XCOMP_ROOT_PATH_SUFFIX ]; then
	if [ -d $TOOLKIT_PATH/aarch64-linux-gnu/libc ]; then
		XCOMP_ROOT_PATH_SUFFIX=aarch64-linux-gnu/libc
	fi
fi

export XCOMP_ROOT_PATH=$TOOLKIT_PATH/$XCOMP_ROOT_PATH_SUFFIX
