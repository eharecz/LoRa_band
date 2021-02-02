#!/bin/sh
#Script to Upload data to ThingSpeak and download data from ThingSpeak.
#The communication between ThingSpeak and Gateway is via http GET/POST. 
#Gateway use curl command to acheive this purpose. 

#How to use: 
#1. Check the section "Write a customized script" of the Gateway User Manual for the data flow of this script
#2. Data format in end node is field1=VALUE[&field2=VALUE]. for example: field1=34.5&field2=86 or field1=35 
#3. If end node send <396640>field1=34.5&field2=85. Gateway will update the channel 396640 with field1=34.5&field2=85

HTTP_URL="https://api.thingspeak.com/"


logger "[IoT]: Start ThingSpeak Process "

CHECK_INTERVAL=5


old=`date +%s`

#Run Forever
while [ 1 ]
do
	now=`date +%s`
	
	
	if [ `expr $now - $old` -gt $CHECK_INTERVAL ];then
		old=`date +%s`
		
		# Check if there is sensor data arrive at /var/iot/channels/ every 5 seconds, if there is one, update to ThingSpeak
		CID=`ls /var/iot/channels/`
		if [ -n "$CID" ];then
			for channel in $CID; do
				data=`cat /var/iot/channels/$channel`			
				logger "[IoT]: Found $data at Local Channel:" $CID
				logger "[IoT]: Update it to server"
				if [ "$channel" == "396640" ];then
					WRITE_API_KEY="P07KVY59P5QEY6M6"
				fi
				
				echo $WRITE_API_KEY
				echo "https://api.thingspeak.com/update?api_key=$WRITE_API_KEY&$data"
				curl -k --get "https://api.thingspeak.com/update?api_key=$WRITE_API_KEY&$data"
				
				#remove this data
				rm /var/iot/channels/$channel
			done
		fi
		
		# Check if there is talkback command in thingspeak
		# see https://thingspeak.com/apps/talkbacks/ 
		talkback=`curl -k https://api.thingspeak.com/talkbacks/30660/commands/execute.json --data "api_key=JZ3X4Y9MTCZNH9YO" 2>/dev/null`
		command=`echo $talkback | awk -F '"' '{print $6}'`
		
		if [[ -n "$command" ]];then
		# Construct Downlink Command and send out
		# {"txpk":{"freq":915.0,"powe":20,"datr":"SF7BW125","codr":"4/5","ipol":false,"data":"$command"}}
			logger "[IoT]: Get downlink command:" $command
			echo "{\"txpk\":{\"freq\":915.0,\"powe\":20,\"datr\":\"SF7BW125\",\"codr\":\"4/5\",\"ipol\":false,\"data\":\"$command\"}}" > /var/iot/push/send
		fi
  else
    sleep 1 # Wait before looping to check time
	fi
done

