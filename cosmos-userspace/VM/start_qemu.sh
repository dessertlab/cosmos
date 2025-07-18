#!/bin/bash

# Check if a path is provided as an argument
if [ $# -ne 1 ]; then
    echo "Usage: $0 <path_to_image_file>"
    exit 1
fi

# Path to the image file
image_file="$1"

# Extract directory from the image file path
image_dir=$(dirname "$image_file")

# Create files in the directory
touch "$image_dir/qemu-monitor-socket"
touch "$image_dir/hyp_logs"

qemu-system-x86_64 \
-drive "file=$image_file,format=qcow2",if=none,id=drive0,format=qcow2 \
-nic user,model=e1000,hostfwd=tcp::2222-:22 \
-device ahci,id=ahci \
-device ide-hd,drive=drive0,bus=ahci.0 \
-cpu host \
-m 2048 \
-smp 2 \
-enable-kvm \
-vnc :0 \
-monitor unix:"$image_dir/qemu-monitor-socket",server=on,nowait \
-daemonize \
-serial file:"$image_dir/hyp_logs"
