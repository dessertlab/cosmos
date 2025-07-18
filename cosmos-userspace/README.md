# COSMOS use guide

## Orchestrator

Create and insert devices: 

```
sudo ./create_devices.sh
```

And insert the modules: 

```
sudo ./ins_cosmos.sh
```

Then you need to compile the userspace program: 

```
make;
```

You can now use the orchestrator: 

```
./cosmos_ui
```

You will be presented with a simple UI giving you the possibilities to: 

1) Set the buffer size of the trace to record; 
2) Reset the buffer; 
3) Start the recording phase; 
4) Read data on buffer traced (use it at the end of the recording);
5) Print a raw seed in a human-readable format. 

You will need a VM containing a target hypervisor to successfully finish a recording. We provide the qemu settings we used to perform the experimental evaluation. You can use it and substitute the qcow2 with the one you prefer. Steps to perform a recording are: 

1) Set the buffer size to the desired one. 
2) Start the recording phase using the orchestrator (set "1" on enable tracing and from which exit to start, "0" on dump_vmcs12 and retrieve_performance).

In another shell: 

3) Start the L1 VM and connects using ssh -p 2222 (remember to correctly modify che qemu script)

```
sudo ./start_qemu.sh <image_qcow2_path>
ssh -p 2222 user@ip (e.g. test@localhost)
```

4) Start the workload (L2 guest) on the VM: it must triggers the hypervisor. 

```
xl create debian7_configuration (e.g. Xen)
```

5) Recording will stop ones reached the preset buffer size (check using "dmesg" on L0 host hypervisor).
6) Read data on buffer in a raw seed (COSMOS UI option 4). 
7) (Optional) Create a human-readable seed (COSMOS UI option 5). 

## APIs

You might want to automate this process without using a UI. In the no_replay folder there is the script we used to automatically perform tests on the target hypervisors. 
It uses the APIs provided by the COSMOS' devices. All usable APIs are: 

```
//COSMOS_Rec
#define SET_BUFFER_SIZE 1
#define RESET 2
#define TRACING_CONTROL 3
#define SET_FROM_EXIT 8
#define SIGNAL_SEM_DUMP_rec 21
#define SET_DUMP_VMCS 22
#define CREATE_EFF_REC_BUFF 23
#define READ_EFF_REC_BUFF 24

//COSMOS_Fi
#define SIGNAL_SEM_DUMP_fi 20
#define SET_DUMP_VMCS 22
#define CONTROL_EXEC 27
#define INJECT_OP 28

//COSMOS_Ut
#define TEST_GUEST_ERROR 18
#define GET_GUEST_ERROR 19
#define DUMP_VMCS12 20
```
