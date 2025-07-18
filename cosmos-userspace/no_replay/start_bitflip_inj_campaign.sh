#!/bin/bash

# usage: ./start_bitflip_inj_campaign guest_cr0 (e.g. 0x30)

hyp="Xen"

sudo pkill qemu;
sleep 1; 

buffer_size=$(cat "./buffer_size")
bit_pos=$(cat "./bit_pos")
idx_op=$(cat "./idx_op")
guest_cr0=$1

sudo ../insmod_Hermes.sh
sudo ../create_device.sh
sudo ../$hyp/avvia_qemu.sh

sleep 1

sudo cosmos_nii_inj $buffer_size $idx_op $bit_pos $guest_cr0
