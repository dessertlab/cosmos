# COSMOS installation guide 

This directory includes the installation guide for all the components of COSMOS and how to apply the patch.
If you want to clone cosmos repository, run the following (if the first cloning) and use you're Gitlab credentials when requested:

```
$ git clone --recurse-submodules https://dessert.unina.it:8088/ldesi/cosmos.git
$ git add . && git commit -m "updates" && git push
```

If you have already cloned cosmos repo be sure to pull every commits in submodules and push (if any) modification:

``git submodule foreach git pull origin master``

## 1. System requirement and kernel installation

* Intel CPU with VT-x enabled.
* minimum 100GB HDD / SSD. 

Information provided has been tested on Ubuntu 22.04.03 LTS, Linux kernel version 5.19.0. 

### Build dependencies

Go to /etc/apt/sources.list and decomment all src lines:

```
sudo nano /etc/apt/sources.list
```

Then update: 

```
sudo apt udpate
```

Install the dependencies and the userspace utilities (Note: on Linux Kernel version 5.19.0, package of Ubuntu 22.04.03 LTS): 

```
sudo apt-get -y build-dep linux linux-source-5.19.0 
sudo apt install qemu linux-source-5.19.0
```

Finally, clone the repo on your local machine. We will need some files later: 

```
git clone https://github.com/dessertlab/Cosmos.git
```

### Kernel compiling and installation

Extract, compile and install the new kernel:

```
sudo cd /usr/src/linux-source-5.19.0; 
sudo tar -xvf linux-source-*;
sudo cd linux-source-5.19.0; 
sudo make menuconfig; 
```

Here just save and exit.

```
sudo nano .config
```

Search for: 

```
CONFIG_LOCAL_VERSION=""
...
CONFIG_SYSTEM_TRUSTED_KEYS="debian/canonical-certs.pem"
CONFIG_SYSTEM_REVOCATION_KEYS="debian/canonical-revoked-certs.pem"
```

And change in: 

```
CONFIG_LOCAL_VERSION="-Cosmos"
...
CONFIG_SYSTEM_TRUSTED_KEYS="../debian/canonical-certs.pem"
CONFIG_SYSTEM_REVOCATION_KEYS="../debian/canonical-revoked-certs.pem"
```

Then (this can last a long time depending on your hardware configuration): 

```
sudo make -j$(nproc); sudo make -j$(nproc) modules_install; sudo make install; 
```

When finished, find the menu entry id option for 'Ubuntu, with Linux 5.19.17-Cosmos' in the grub.cfg file: 

```
sudo cat /boot/grub/grub.cfg
```

Then (substitute ID_ENTRY with the one found in grub.cfg): 

```
sudo sed -i 's/GRUB_DEFAULT.*/GRUB_DEFAULT=ID_ENTRY/' /etc/default/grub;
sudo update-grub;
sudo reboot; 
```

You can check the new kernel with:

```
uname -r
```

And it should be "5.19.17-Cosmos". 

## 2. COSMOS installation 

Now you can apply the patch under the /usr/src/linux-source-5.19.0 directory: 

```
cd /usr/src/linux-source-5.19.0/linux-source-5.19.0;
sudo cp -r path/to/cosmos/all/Kernelspace/vmx/cosmos /usr/src/linux-source-5.19.0/linux-source-5.19.0/arch/x86/kvm/vmx;
sudo cp path/to/cosmos/all/Kernelspace/Makefile /usr/src/linux-source-5.19.0/linux-source-5.19.0/arch/x86/kvm/;
sudo cp path/to/cosmos/all/Kernelspace/vmx/cosmos_all.patch /usr/src/linux-source-5.19.0/linux-source-5.19.0/arch/x86/kvm/vmx;
cd arch/x86/kvm/vmx;
patch -p0 < cosmos_all.patch; 

cd /usr/src/linux-source-5.19.0/linux-source-5.19.0;

sudo make modules -j6  SUBDIRS=arch/x86/kvm/;
sudo make -j6  M=arch/x86/kvm/;

sudo cd /usr/src/linux-source-5.19.0/linux-source-5.19.0/arch/x86/kvm/vmx/cosmos;
sudo make; 
```

### Post installation 

To use the orchestrator with COSMOS, follow the guide under cosmos_userspace: 

### Uninstallation

If you want to uninstall COSMOS, follow the following steps: 

```
sudo apt-get purge linux-source-5.19.0; 
```

If you want to clean up everything, remove the related files and directories under: 

```
/boot
/etc or /boot/grub
/lib/modules/KERNEL-VERSION/* (/lib/modules/$(uname -r)
```

### Troubleshooting

Here we will list all the problems we faced into compiling the linux source kernel and you could run into too. 

#### Make modules_install or make install errors

This problem could be related to some errors during the "sudo make" phase. We advice you to run the make command without the flag -j to understand if and where the errors are. 

A common issue regarding this problem is the missing or the wrong path of the certificated needed to compile the kernel. Go to: 

```
sudo cd /usr/src/linux-source-5.19.0/linux-source-5.19.0;
nano .config
```

Locate the line "CONFIG_SYSTEM_TRUSTED_KEYS" (CTRL + W if you are using nano). Change the line according to the path of the certs of your kernel version (e.g. CONFIG_SYSTEM_TRUSTED_KEYS="../debian/canonical-certs.pem"), do the same for the line CONFIG_SYSTEM_REVOCATION_KEYS (e.g. CONFIG_SYSTEM_REVOCATION_KEYS="../debian/canonical-revoked-certs.pem"). 

If you do not have the certs folder, **do not remove the lines** but just modify them with empty strings (e.g. CONFIG_SYSTEM_TRUSTED_KEYS="" and CONFIG_SYSTEM_REVOCATION_KEYS="").

After that, rerun: 

```
sudo make; 
sudo make modules_install; 
sudo make install; 
```

#### Errors during the compilation of the linux kernel modules

There can be many reasons for this problem. Remember to: 

    - ``Install the headers``:  sudo apt-get install linux-headers-5.19.0-50-generic linux-hwe-5.19-headers-5.19.0-50


## Citation

If you use **COSMOS** in your research or project, please cite our paper (accepted at IEEE TDSC 2025):

TO_ADD

## License

This tool is released under the **MIT License**, allowing free use, modification, and distribution, provided proper attribution is given.

