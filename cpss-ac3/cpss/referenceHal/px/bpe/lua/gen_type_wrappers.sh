#!/bin/sh

D=`dirname $0`
C=`(cd $D/../../../..;pwd)`
L=$C/mainLuaWrapper
O=${O:-$C/compilation_root}
if [ -f $O/appDemoSim -a ! -f $O/type_info_file ]; then
    gdb $O/appDemoSim > $O/type_info_file <<eof
info types
quit
eof
fi
if [ ! -f $O/type_info_file ]; then
    echo "no $C/compilation_root/type_info_file"
fi

python $L/scripts/make_type_wrapper.py \
	-i $O/type_info_file \
	-p $L/scripts/already_implemented.lst \
	-X $L/scripts/excludelist \
	-X $D/types_excludelist \
	-C $C \
	-M $L/scripts/lua_C_mapping \
	-N -F NONE -I referenceHal \
	-o $D/wrappers/all_refhal_lua_c_type_wrappers.c
