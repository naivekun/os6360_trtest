#/bin/sh
#
# Pack Lua scripts for referenceHal

C=`dirname $0`
S=$C/../../../../mainLuaWrapper/scripts

$S/cmdFS_RAM_embed_files.py \
    -z .lua -z .xml \
    -S 1000000 \
    -E _REFHAL \
    -o $C/data/cmdFS_RAM_embed_files_refHal.c \
    -D $C/scripts
