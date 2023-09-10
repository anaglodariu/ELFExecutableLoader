# ELF Executable Loader
Objectives
--
- delve into how an executable is loaded and run by the Operating System
- getting used to working with **memory exceptions** on an OS
- using the Linux API to work with **address space, virtual memory and demand paging**

Overview
--
Implementing an **ELF loader for Linux** as a **shared/dynamic library**. The loader
will load the executable into memory page by page using a demand paging mechanism 
(a page will only be loaded when it is needed). For simplicity, the loader will only run
**static executables**, which are not linked with shared/dynamic libraries.

Representation of a segment
--
![so_seg](https://github.com/anaglodariu/ELFExecutableLoader/assets/94357049/e3b2f9e2-9dfb-42e3-8832-288421cbc37a)
