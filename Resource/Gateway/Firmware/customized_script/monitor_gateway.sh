#!/bin/sh
#This script is used to monitor gateway status. 

SERVER='mqtt.thingspeak.com'
USER='xxxx'             # MQTT User
PASS='xxxxx'     #MQTT_API_KEY

CHAN_ID='xxxxxxxxxxxxxx'    #Channel ID
CHAN_KEY='xxxxxxxxxxxxxx'   #Channel Write API



HOSTNAME=`uci get system.@[0].hostname`

time_now=`date`

uptime=`uptime`

cpu_load_1m=`echo ${uptime#*"load"} | awk '{print $2}'`


ram_free_k=`free -b -n 1 -d 0 | grep Mem | head -n 1 |awk '{print $4}'`
ram_free_M=$(expr $ram_free_k / 1024 )


disk_used=`du -h / -d 1 | tail -n 1 |awk '{print $1}' |cut -f 1 -d "M"`


has_day=`uptime | grep "day" -c`
has_min=`uptime | grep "min" -c`

if [ "$has_day" == "1" ] 
then
    if [ "$has_min" == "1" ]; then
		uptime_d=`uptime | awk '{print $3}'`
		uptime_m=`uptime | awk '{print $5}'`
		uptime_m=$(expr $uptime_d \* 1440 + $uptime_m)
	else
		uptime_d=`uptime | awk '{print $3}'`
		uptime_h=`uptime | awk -F '[ ]+|[:,]' '{print $9}'`
		uptime_m=`uptime | awk -F '[ ]+|[:,]' '{print $10}'`
		uptime_m=$(expr $uptime_d \* 1440 + $uptime_h \* 60 + $uptime_m)	
	fi
else 
    if [ "$has_min" == "1" ]; then
		uptime_m=`uptime | awk '{print $3}'`

	else
		uptime_h=`uptime | awk -F '[ ]+|[:,]' '{print $6}'`
		uptime_m=`uptime | awk -F '[ ]+|[:,]' '{print $7}'`
		uptime_m=$(expr $uptime_h \* 60 + $uptime_m)	
	fi
fi

	
online_offline=`cat /var/iot/status`
	
if [ "$online_offline" = "online" ]
then
    {
		online_offline_1_0=1
	}
elif [ "$online_offline" = "offline" ]
then
    {
		online_offline_1_0=0
	}
fi

cfg_ver=`uci get system.@system[0].config_ver`

PUBLIC_IP=`curl ifconfig.me -m 10`	

upload_data="field1="$cpu_load_1m"&field2="$ram_free_M"&field3="$disk_used"&field4="$uptime_m"&field5="$online_offline_1_0"&field6="$cfg_ver"&field7="$PUBLIC_IP"&status=MQTTPUBLISH"
mosquitto_pub -h $SERVER -p 1883 -u $USER -P $PASS  -i $HOSTNAME -t channels/$CHAN_ID/publish/$CHAN_KEY -m $upload_data

echo $upload_data
