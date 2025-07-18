#! /usr/bin/bash

cd /usr/src/linux-source-5.19.0/linux-source-5.19.0/arch/x86/kvm/vmx/cosmos/
make 

cd /usr/src/linux-source-5.19.0/linux-source-5.19.0
sudo rmmod -f cosmos_utilities; sudo rmmod kvm_intel; sudo rmmod kvm; sudo rmmod cosmos_fi; sudo rmmod cosmos_rec;
sudo insmod arch/x86/kvm/kvm.ko; 
sudo insmod arch/x86/kvm/vmx/cosmos/cosmos_rec.ko
sudo insmod arch/x86/kvm/vmx/cosmos/cosmos-fi.ko
sudo insmod arch/x86/kvm/kvm-intel.ko
sudo insmod arch/x86/kvm/vmx/cosmos/cosmos_utilities.ko
