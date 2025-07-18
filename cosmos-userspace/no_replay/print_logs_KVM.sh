#!/bin/sh

# Check if all parameters have been passed
if [ "$#" -ne 5 ]; then
  echo "Usage: $0 WORKLOAD_NAME exit_reason field bit_pos rep"
  exit 1
fi

WORKLOAD_NAME="$1"
exit_reason="$2"
field="$3"
bit_pos="$4"
rep="$5"

# Execute the command with timeout
timeout 2s sshpass -p "test" ssh -p 2222 root@localhost "/home/test/print_logs.sh"

# Check if the command has timed out
if [ $? -eq 124 ]; then
  # If the timeout has expired, create the file with the value 1
  echo "1" > "./tests/${WORKLOAD_NAME}/${exit_reason}/${WORKLOAD_NAME}${field}/bit${bit_pos}/rep_${rep}/l1_crashed"
  exit 1
fi

# Check if the "qemu" process is NOT running
sshpass -p "test" ssh -p 2222 root@localhost "pgrep qemu > /dev/null"
if [ $? -ne 0 ]; then
  echo "1" > "./tests/${WORKLOAD_NAME}/${exit_reason}/${WORKLOAD_NAME}${field}/bit${bit_pos}/rep_${rep}/guest_crashed"
fi


