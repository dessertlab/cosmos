#!/bin/bash

while true
do
    # Uccidi il processo qemu
    sudo pkill qemu

    # Attendi 2 secondi
    sleep 2

    # Controlla se il processo qemu è ancora attivo
    if ! ps aux | grep -q "[q]emu"; then
        break  # Esce dal loop se il processo non è più attivo
    fi
done
