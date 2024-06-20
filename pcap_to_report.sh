echo "###############################################################"
echo "########### Ricordati di cancellare il db su rita #############"
echo "###############################################################"

sudo rm extracted_logs/*
pcap_filename="$1"

container_id=$(sudo docker run -itd zeek/zeek)
echo "Starto il container $container_id"
pcap_filename=$(basename $pcap_filename)
echo "Copiando il file '$pcap_filename' nel container"
sudo docker cp $1 $container_id:/home/$pcap_filename

echo "Eseguendo: zeek -r /home/$pcap_filename"
sudo docker exec -it $container_id zeek -r /home/$pcap_filename > /dev/null 2>&1

echo "Eseguendo mv *.log /home"
sudo docker exec -it $container_id sh -c 'mv *.log /home/' > /dev/null 2>&1

echo "Copiando i file nella home nella cartella (locale) /extracted_logs"
sudo docker cp $container_id:/home/. ./extracted_logs > /dev/null 2>&1

echo "Cancello il container"
sudo docker stop $container_id > /dev/null 2>&1
sudo docker rm $container_id > /dev/null 2>&1

echo "Creo report di rita"
filename=$(echo "$pcap_filename" | sed 's/[\/\\.<>"*<>:|?$ ]//g')
sudo rita import extracted_logs/ $filename
sudo rita html-report $filename
