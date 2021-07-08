#!/usr/bin/env bash
if [ "$1" = "" ]; then
	echo "Usage:"
	echo "    $0 iDebug.xml"
	exit 0
fi
xsd=`dirname $0`/iDebug.xsd
if [ \! -f $xsd ]; then
	echo "Error: $xsd not found"
	exit 1
fi
xmllint --schema $xsd $1 >/dev/null
