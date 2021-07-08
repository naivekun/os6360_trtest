#!/bin/bash

len=$(stat -c %s $1)
var=$(xxd -p -c $len $1)
sum8=0;
for (( i=0; i<$len*2; i+=2 ));
do
	sum8=$(( $sum8 + 0x${var:$i:2} ))
done
sum8=$(printf "%x" $(( $sum8 & 0xFF )))
echo $sum8
exit 0
