pcap_filename="$1"
container_id=$(sudo docker run -itd zeekurity/zeek)

sudo docker cp $1 $container_id:/home/$pcap_filename

pcap_filename=$(basename $pcap_filename)

sudo docker exec -it $container_id zeek -r /home/$pcap_filename > /dev/null 2>&1

sudo docker exec -it $container_id sh -c 'mv *.log /home/' > /dev/null 2>&1

sudo docker cp $container_id:/home/. ./extracted_logs > /dev/null 2>&1

sudo docker stop $container_id > /dev/null 2>&1
sudo docker rm $container_id > /dev/null 2>&1

filename=$(echo "$pcap_filename" | sed 's/[\/\\.<>"*<>:|?$ ]//g')
sudo rita import extracted_logs/ $filename
sudo rita html-report $filename
