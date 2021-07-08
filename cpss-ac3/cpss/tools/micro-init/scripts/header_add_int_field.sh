#!/bin/bash
#################################################################
# INPUTS:       						#
# $1 - header txt file						#
# $2 - field length 1=U8, 2=U16, 4=U32				#
# $3 - field value - the data that need to be inserted          #
#		to the field padding will be added until field  #
#		length				                #
#################################################################
	
TMP_FILE=$1
FIELD_LEN=$2
FIELD_VALUE=$3

case $FIELD_LEN in
	 1 )
		printf -v FIELD_VALUE '%02x\n' "$FIELD_VALUE"
	   ;;
	 2 )
		printf -v FIELD_VALUE '%04x\n' "$FIELD_VALUE"
	   ;;
	
	 4 )
		printf -v FIELD_VALUE '%08x\n' "$FIELD_VALUE"
	   ;;
	 8 )
		printf -v FIELD_VALUE '%016x\n' "$FIELD_VALUE"
	   ;;
	
	 *)
	   echo "############################################################"
	   echo "####           ERROR - invalid field length.            ####"
	   echo "############################################################"
	   return 0
esac 
echo -n ${FIELD_VALUE} >> ${TMP_FILE}
