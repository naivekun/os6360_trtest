#!/bin/bash
#################################################################
# INPUTS:      			        			#
# $1 - header txt file						#
# $2 - field length - num of chars in field			#
# $3 - field value - the data that need to be inserted          #
#		to the field padding will be added until field  #
#		length						#
#################################################################	

TMP_FILE=$1
FIELD_LEN=$2
FIELD_VALUE=$3

# get field length
version_len=${#FIELD_VALUE}
if [ $version_len -gt $FIELD_LEN ]
then
	echo "################################################################"
	echo "####          Error - invalid parameter length.             ####"
	echo "####"  	       $version_len ">" $FIELD_LEN	         "####"
	echo "################################################################"
	return 0 
else
	# convert string to ascii and remove unwanted chars
	FIELD_VALUE=$(echo ${FIELD_VALUE} | xxd -g 1 |sed -e 's/[0-9]*: //g' -e 's/[^ ]*$//g')
	FIELD_VALUE=$(echo ${FIELD_VALUE} |sed -e 's/ //g' -e 's/0a$//')
	echo -n ${FIELD_VALUE} >> $TMP_FILE

fi
COUNTER=$version_len
while [  $COUNTER -lt $FIELD_LEN ]; do
		echo -n '00' >> $TMP_FILE
		let COUNTER=COUNTER+1 
done
