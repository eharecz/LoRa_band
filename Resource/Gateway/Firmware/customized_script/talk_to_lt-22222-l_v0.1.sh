#!/bin/sh
# This scripts shows how to use LPS8/LG308/DLOS8 to communicate with two LoRaWAN End Nodes, without the use of internet or LoRaWAN server
#
# Hardware Prepare: 
# 1. LT-22222-L x 2, both are configured to work in 
#   a) Class C ; 
#	b) ABP Mode ; 
#	c) AT+Mod=1
# 2. LPS8, 
#   a) Firmware version > 
#   b) Lorawan server choose built-in
#   c) in Custom page, select custom script to point to this script. (put this script in /etc/iot/scripts directory)
# 
# How it works? 
#   a) Devices 1 sends a uplink payload to LPS8. LPS8 will get the DI1 and DI2 info from the payload
#   b) LPS8 will send a message to Device 2 to set the Device2 DO1 = Device1 DI1, and Device DO2 = Device DI2.
#   c) Device2 will change DO1 and DO2 to according to the message from LPS8, and send back a message to LPS8 with the its DO1 and DO2 value. LPS8 will ask Device1 to change its DO1 to same as Device 2, and change the DO2 to the same as Device 2. ( The purpose of this step is to show that the Device2 has already do the change there). 
# 
#  For example: If current status of Device1 and Device2 leds shows: 
#  Device1: DI1: ON, DI2: ON , DO1: OFF,  DO2: OFF
#  Device2: DI1: OFF, DI2: OFF , DO1: OFF,  DO2: OFF
#
#  Step2  will cause below change: 
#  Device1: DI1: ON, DI2: ON , DO1: OFF,  DO2: OFF
#  Device2: DI1: OFF, DI2: OFF , DO1: ON,  DO2: ON
#  
#  Step3 will cause below change: 
#  Device1: DI1: ON, DI2: ON , DO1: ON,  DO2: ON
#  Device2: DI1: OFF, DI2: OFF , DO1: ON,  DO2: ON
#  So if a person is in the Device 1 location, he can check if the DO LED match DI LEDs on Device 1 to confirm whether the Device 2 has been changed. 

DEV_1=$1   #INPUT DEVICE
DEV_2=$2   #OUTPUT DEVICE
Time2=`date +%s`

inotifywait -mrq -e 'create,delete,close_write,attrib,moved_to' /var/iot/channels/ | while read event
#Process below loop once there is an event arrives
do 
	#Print out this event
	echo "            "
	echo "############New Event###########"
	echo $event
	
	#Get Dev Address from this event
	DEV_ADDR=`echo $event | awk '{print $3}'`
	
	#   This is an event from Dev_1. 
    #	DEV_1 send message to control DEV_2
	if [ "$DEV_ADDR" = "$DEV_1" ]  ;then
		Time1=`date +%s`
		# Two events time of DEV_1 should be more than 10 seconds, to avoid endless loop. 
		if [ `expr $Time1 - $Time2` -gt 10 ];then
			# Process for Dev_1
			# Print Payload from Dev_1 
			PAYLOAD1=`hexdump -v -e '1/1 "%.2x"'  -n 11 /var/iot/channels/$DEV_1`
			DIDO_BYTE=`xxd -b /var/iot/channels/$DEV_1 | grep "00000006" | awk '{print $4}'`    ## Get DIDO Bit	, check LT-22222-L User manual 
			DI1=`echo ${DIDO_BYTE:4:1}`        ## Get DI1 value
			DI2=`echo ${DIDO_BYTE:3:1}`   		## Get DI2 value	
			DO1=`echo ${DIDO_BYTE:7:1}`           ## Get DO1 value
			DO2=`echo ${DIDO_BYTE:6:1}`           ## Get DO2 value
			echo "Payload: 0x$PAYLOAD1"
			echo "DIDO BYTE: 0x${DIDO_BYTE} : DI1:$DI1 DI2:$DI2 DO1:$DO1 DO2:$DO2"
			echo ""
			
			#Compose the Payload to be sent
			echo "Set ${DEV_2} to DO1: $DI1, DO2: $DI2"
			echo "Sent: 020${DI1}0${DI2}00"                      # See Downlink Payload From LT-22222-L Manual Type Code 0x02
			echo "${DEV_2},imme,hex,020${DI1}0${DI2}00" > /var/iot/push/down         # Put the downlink payload in queue to send. 
		fi
	fi
	
	# Got the confirm message from DEV_2 and set DEV_1 DO. 
	if [ "$DEV_ADDR" = "$DEV_2" ];then
		# Process for Dev_2
		# Print Payload from Dev_1 
		PAYLOAD2=`hexdump -v -e '1/1 "%.2x"'  -n 11 /var/iot/channels/$DEV_2`
		DIDO_BYTE=`xxd -b /var/iot/channels/$DEV_2 | grep "00000006" | awk '{print $4}'`    ## Get DIDO Bit	
		DI1=`echo ${DIDO_BYTE:4:1}`        ## Get DI1 value from dev_2
		DI2=`echo ${DIDO_BYTE:3:1}`   		## Get DI2 value	
		DO1=`echo ${DIDO_BYTE:7:1}`           ## Get DO1 value
		DO2=`echo ${DIDO_BYTE:6:1}`           ## Get DO2 value
		echo "Payload: 0x$PAYLOAD1"
		echo "DIDO BYTE: 0x${DIDO_BYTE} : DI1:$DI1 DI2:$DI2 DO1:$DO1 DO2:$DO2"
		echo ""
		
		#Compose the Payload to be sent
		echo "Sent: 020${DO1}0${DO2}00"                        # See Downlink Payload From LT-22222-L Manual Type Code 0x02
		echo "${DEV_1},imme,hex,020${DO1}0${DO2}00" > /var/iot/push/down      # Put the downlink payload in queue to send to Dev_1       
		echo "Set ${DEV_1} to DO1: $DO1, DO2: $DO2"
		Time2=`date +%s`
	fi
done 