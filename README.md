# cartelle

- rita
  
  - Contiene il source del programma (tuttavia e' installato sul sistema basta eseguire `sudo rita ...`)

- dns_covert_tools
  
  - Contiene gli eseguibili dei tool per l'offuscamento

- pcaps
  
  - Contiene i file pcap generati dalle varie prove utilizzando i diversi tool

- CobaltStrike
  
  - Contiene l'eseguibile di CobaltStrike (utilizzato per generare dei beacon di prova)

- test_funzionamento_mongodb
  
  - Contiene ```./provaconnessione``` che e' utile a testare il funzionamento di mongo (in caso rita non si connette) e il file start_mongo per avviare una nuova istanza docker di mongo.

- extracted_logs
  
  - contiene i risultati ottenuti lanciando il comando ```./pcap_to_report file.pcap ```
  
  ---
  
  # Files
  
  - pcap_to_report.sh: gli si da in pasto un pcap e genera un report html
  
  - link_to_report.sh: simile a pcap_to_report.sh, ma gli si da direttamente un link di un pcap di un malware dal sito: [Malware-Traffic-Analysis.net](https://www.malware-traffic-analysis.net/)
  
  - conf.sh: apre il file di configurazione di rita in vim
  
  - *.py: sono file realizzati prima di provare i tool gia' costruiti per l'offuscamento, lasciati in caso possano rimanere utili per prove manuali sui parametri dns.
