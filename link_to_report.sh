#!/bin/bash

if [ -z "$1" ]
then
    echo "Inserisci il link: "
    read link
else
    link=$1
fi

filename=$(basename $link)

container_id=$(sudo docker run -itd zeekurity/zeek)

echo -ne '[##                 ] (15%)\r'
sudo docker exec -it $container_id apt-get update > /dev/null 2>&1

echo -ne '[#####              ] (33%)\r'
sudo docker exec -it $container_id apt-get install -y wget unzip > /dev/null 2>&1

echo -ne '[#############      ] (66%)\r'
sudo docker exec -it $container_id wget $link -P /home > /dev/null 2>&1

sudo docker exec -it $container_id unzip -P infected /home/$filename -d /home > /dev/null 2>&1

pcap_filename="${filename%.*}"

sudo docker exec -it $container_id zeek -r /home/$pcap_filename > /dev/null 2>&1

echo -ne '[###################] (100%)\r'
sudo docker exec -it $container_id sh -c 'mv *.log /home/' > /dev/null 2>&1

sudo docker cp $container_id:/home/. ./extracted_logs > /dev/null 2>&1

sudo docker stop $container_id > /dev/null 2>&1
sudo docker rm $container_id > /dev/null 2>&1

filename=$(echo "$filename" | sed 's/[\/\\.<>"*<>:|?$ ]//g')
sudo rita import extracted_logs/ $filename 
sudo rita html-report $filename

echo -ne '\n'
