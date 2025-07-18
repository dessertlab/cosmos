#!/bin/bash

sudo pkill qemu; 

while true; do
  if sudo /home/test/test_snapshot/$1/avvia_qemu.sh 2>&1 | grep -q "Failed to get \"write\" lock"; then
    echo "Il file di immagine è bloccato da un altro processo. Riavvio QEMU..."
  else
    echo "QEMU avviato con successo. Uscita dal ciclo."
    break
  fi
  sleep 1  # Attendi un secondo prima del prossimo controllo
done
