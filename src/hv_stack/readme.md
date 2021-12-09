# NoirPvCuda User Hypervisor

## The vCPU Scheduler Algorithm
To be efficient with paravirtualized kernel, vCPU must have its own vCPU scheduler algorithm as well.

## Emulation of Processor Halt
Any operating system will halt the processor as there are no threads ready to run. Paravirtualized kernel will, like any other normal OSes, execute `hlt` instruction to schedule away the vCPU. \
According to x86 architecture, the processor will resume execution from halted state once an interrupt takes place.

## Emulation of Interrupts
An interrupt is subject to be injected once there is an external event to be passed to the virtual machine. The types of external events includes:

- Asynchronous File I/O completes.
- Timer is due.
- Console (Standard Input) has inputs ready.

The design for the emulation of interrupts follows the standard design of interrupts. \
There are two types components: centralized thread of interrupt requestor, and threads of emulated external hardwares.

### Centralized Thread of Interrupt Requestor
The centralized thread of interrupt requestor waits for all emulated hardware threads. \
When there is a thread has completed an event, an interrupt will be requested. \
The centralized thread of interrupt requestor will signal vCPUs subject to be injected with the event.

### Thread of Asynchronous File I/O
The asynchronous File I/O is a feature that enables multi-tasking feature of a virtual processor to do read/write access asynchronously. When a vCPU issues a hypercall to read or to write a file, the I/O request will be sent to a queue that holds all asynchronous File I/O. \
The asynchronous File I/O queue will be multi-producer-single-consumer model. The producers are the vCPUs, and the consumer is the thread that performs the asynchronous File I/O. \
When the asynchronous File I/O thread completes the request, the thread would request an interrupt to the vCPU that issued the hypercall. The request is sent to the centralized thread of interrupt requestor.

### Thread of Timer Hardware
The timer hardware feature allows the paravirtualized kernel to slice the CPU time. When the timer is due, the timer interrupt will be broadcasted to all vCPUs.

### Thread of Console Input
The thread of console input allows the Guest to access virtual console inputs. In that console input requires waiting, getting data from console is designed to be asynchronous. When the data from console is ready, an interrupt will be sent to the Guest.

## Control-Flow Design
To emulate external interrupts, not only should the developer master the use of event injection, but the developer should also master how to intercept interrupt windows. An interrupt window would allow further delivery of interrupts, so when an interrupt window occurs, an queued external event should be dequeued and injected to the Guest.

## Interrupt Logic
A vCPU can be either scheduled in (running on a logical core) or scheduled out (staying halted). An event can only be injected if the vCPU is scheduled out. Therefore, cancel the execution of vCPU if scheduled in.

If a vCPU enters a halted state via `hlt` instruction, it should call a synchronous function to wait for any signals from virtual hardwares.

If a vCPU enters a state capable of taking interrupts after an interrupt is taken, a VM-Exit of interrupt-window will issued. The vCPU should check if there is pending interrupts and get them injected serially.

## File I/O Paravirtualization
This project would adopt a file path system similar to the one of Linux:

- The `/` directory is the root directory. The location of the root directory on the host is subject to be configured by User Hypervisor.
- The `/mnt/` directory is the mount directory. User Hypervisor can configure other directories on the host to be accessible by Guest via mounting them.
- The `/dev/` directory is the device directory. Paravirtualized devices are located in this directory. Layout in this directory should follow the one in a real Linux machine.