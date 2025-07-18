#!/bin/bash

sudo pkill qemu;
contatore=0;

while true; do
  if sudo /home/test/sync/insmod_Hermes.sh 2>&1 | grep -q "Module kvm_intel is in use"; then
    echo "Il file di immagine è bloccato. Riprovo ad inserire Hermes...";
    contatore+=1;

    if [ "$contatore" -eq 6 ]; then
	sudo pkill qemu;
	sshpass -p "test" ssh giuseppe@10.9.0.10 "echo 1 > /home/giuseppe/check.txt";
    fi
  else
    echo "Hermes inserito con sucesso. Uscita dal ciclo."
    break
  fi
  sleep 1  # Attendi un secondo prima del prossimo controllo
done
