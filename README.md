# NoirPvCuda
Paravirtualized Cuda Platform via NoirVisor CVM

## Introduction
This project is a set of modules that provides Cuda ParaVirtualization via [NoirVisor CVM](https://github.com/Zero-Tang/NoirCvmApi).

This is a crossover project of [NoirVisor](https://github.com/Zero-Tang/NoirVisor) and [PaddlePaddle](https://github.com/PaddlePaddle).

## Components
There are four components in this project:

- User Hypervisor
- Guest Console
- GpuOpManager
- Paravirtualized Kernel

### User Hypervisor
The User Hypervisor hosts the Guest by providing a series of paravirtualized hardwares including timer, file I/O, etc. \
User-Hypervisor would also provide a JTAG-level debugging agent to aid the development of Paravirtualized Kernel.

### Guest Console
The Guest Console is a separate program that uses pipelining-over-named-pipes algorithm in order to virtualize the Console Hardware for Guest.

### GpuOpManager
The GpuOpManager is a GPU scheduler that aims to provide best performance for high-priority deep-learning jobs.

### Paravirtualized Kernel
The Paravirtualized Kernel is a minimal OS kernel that relies on resources provided by User Hypervisor and dispatches resources to processes in Guest.

## License
This project is licensed under the Apache 2.0 License.