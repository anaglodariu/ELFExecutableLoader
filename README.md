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
To run an executable, the loader will perform the following steps:
- it will initialize its internal structures
- it will parse the binary file (ELF file parser on Linux)
- it will run the first instruction of the executable (the entry point)
- during execution, a page fault will be generated for each access to a non-mapped page in memory
- it will detect each access to an unmapped page and check which segment of the executable it belongs to:
  - if it is not found in a segment, it means that it is an invalid memory access, therefore the default page fault handler is run
  - if the page fault is generated in an already mapped page, then an unauthorized memory access is attempted -> the default page fault handler is run
  - if the page is found in a segment and it has yet been mapped, then it is mapped to the related address with the permission fo that segment
- the **mmap** function will be used to allocate virtual memory

Specific notions and concepts used
--
- address space
- page access permissions
- the format of executable files
- demand paging
- page fault
- file mapping

Representation of a segment
--
![so_seg](https://github.com/anaglodariu/ELFExecutableLoader/assets/94357049/e3b2f9e2-9dfb-42e3-8832-288421cbc37a)
