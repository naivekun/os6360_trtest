#!/bin/bash
#
# Example:
#    compare_apis.sh ~/cpss_4.0_276 ~/cpss_4.1_059 output2.txt

usage() {
	echo "Usage: $0 cpss_4.0_path cpss_4.1_path output2.txt"
	exit 0
}

test -d "$1/." || usage
test -d "$2/." || usage
test -f "$3" || usage

cpss40=$1
cpss41=$2
input=$3

tool_dir=`dirname $0`

# get the list of all APIs used
sed 's/--.*$//' $input |
	sed -n 's/^[^"]*"\([^"]*\)".*$/\1/p' |
	sort -u > apis_used.txt

python $tool_dir/gen_lua_funcdescr.py -C $cpss40 -o cpss40_desc.lua `cat apis_used.txt`
python $tool_dir/gen_lua_funcdescr.py -C $cpss41 -o cpss41_desc.lua `cat apis_used.txt`

diff -u -F '^cpssAPIs' cpss40_desc.lua cpss41_desc.lua > cpssAPIs.diff
