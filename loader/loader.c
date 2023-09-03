/*
 * Loader Implementation
 *
 * 2022, Operating Systems
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h> /*library for mmap*/
#include <fcntl.h> /*library for open*/
#include <unistd.h> /*library for close*/

#include "exec_parser.h"
#include "signal.h" /*library for sigaction*/


static so_exec_t *exec;
static char *file_path; /*imi retin calea executabilului*/
static int ok1 = 1;

static void segv_handler(int signum, siginfo_t *info, void *context)
{
	void *addr = info->si_addr; /*adresa in memorie la care imi este generat un pagefault*/
	int fd = open(file_path, O_RDONLY);
	int segment_number = 0; /*retin in ce segment imi este generat pagefaultul*/
	int ok = 0;
	for (int i = 0 ; i < exec->segments_no ; i++) {
		unsigned int mem_size = exec->segments[i].mem_size;
		uintptr_t virtual_address = exec->segments[i].vaddr;

		if (ok1 == 1) {
			/*intru in acest if o singura data, fac ok1 = 0 la finalul for-ului*/
			int nr_pagini = 0;
			/*iau in calcul paginile fiecarui segment pana in mem_size*/
			if (mem_size % getpagesize() == 0)
				nr_pagini = mem_size / getpagesize();
			else
				nr_pagini = mem_size / getpagesize() + 1;
			/*la inceput nicio pagina nu este mapata*/
			exec->segments[i].data = (void *) calloc(nr_pagini, sizeof(int));
		}
		if (virtual_address + mem_size > (uintptr_t)addr && virtual_address <= (uintptr_t)addr) {
			segment_number = i;
			ok = 1;
		}
	}
	ok1 = 0;
	if (ok == 0) {
		/*adresa la care imi este generat pagefaultul se afla dupa mem_size, nu este intr-un segment cunoscut*/
		close(fd);
		/*actiunea default asociata semnalului SIGSEGV va fi aplicata*/
		signal(SIGSEGV, SIG_DFL);
	}
	void *p = NULL;
	unsigned int perm = exec->segments[segment_number].perm;
	unsigned int file_size = exec->segments[segment_number].file_size;
	unsigned int mem_size = exec->segments[segment_number].mem_size;
	unsigned int offset = exec->segments[segment_number].offset;
	uintptr_t virtual_address = exec->segments[segment_number].vaddr;
	int *data = (int *) exec->segments[segment_number].data;
	/*indexul paginii la care imi este generat pagefaultul in cadrul segmentului cu indicele "segment_number"*/
	int index_pagina = ((uintptr_t)addr - virtual_address)/getpagesize();
	int write_from_file = file_size - index_pagina * getpagesize(); /*determina cat am de scris din executabil*/
	int write_from_offset = offset + index_pagina * getpagesize(); /*de la ce offset scriu din executabil*/
	int pages_written_from_exec = 0; /*numarul total de pagini in care voi avea scrise date din executabil*/
	/*adresa paginii de la care mapam*/
	void *mapped_from_address = (void *)(virtual_address + index_pagina * getpagesize());

	if (file_size % getpagesize() == 0)
		pages_written_from_exec = file_size / getpagesize();
	else
		pages_written_from_exec = file_size / getpagesize() + 1;
	if (data[index_pagina] == 0) {
		/*daca pagina nu este mapata*/
		if (virtual_address + pages_written_from_exec * getpagesize() > (uintptr_t)addr && virtual_address <= (uintptr_t)addr) {
			/*daca adresa la care este generat pagefaultul este intr-o pagina care trebuie sa contina date din executabil*/
			if (write_from_file >= getpagesize()) {
				/*daca toata pagina trebuie sa contina date din executabil*/
				p = mmap(mapped_from_address, getpagesize(), PROT_WRITE, MAP_FIXED | MAP_PRIVATE, fd, write_from_offset);
			} else {
				/*daca doar o parte din pagina se completeaza cu date din executabil*/
				p = mmap(mapped_from_address, write_from_file, PROT_WRITE, MAP_FIXED | MAP_PRIVATE, fd, write_from_offset);
				if (mem_size > file_size) {
					/*daca am si bss, partea ramasa necompletata din pagina trebuie zerorizata*/
					if (mem_size - file_size < getpagesize() - write_from_file)
						memset((void *)(virtual_address + file_size), 0, mem_size - file_size);
					else
						memset((void *)(virtual_address + file_size), 0, getpagesize() - write_from_file);
				}
			}
		} else {
			/*avem pagini doar cu zero*/
			p = mmap(mapped_from_address, getpagesize(), PROT_WRITE, MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
			/*paginile se mapeaza cu flagul anonymous, nu mai copiem date dintr-un fisier, continutul va fi initializat cu 0*/
		}
		data[index_pagina] = 1; /*pagina a fost mapata*/
		mprotect(p, getpagesize(), perm); /*pagina mapata are permisiunile segmentului*/
		close(fd);
	} else {
		/*acces nepermis la memorie, pagina deja mapata*/
		close(fd);
		signal(SIGSEGV, SIG_DFL);
	}
}

int so_init_loader(void)
{
	int rc;
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = segv_handler;
	sa.sa_flags = SA_SIGINFO;
	rc = sigaction(SIGSEGV, &sa, NULL);
	if (rc < 0) {
		perror("sigaction");
		return -1;
	}
	return 0;
}

int so_execute(char *path, char *argv[])
{
	file_path = path; /* cale executabil pentru functia open*/
	exec = so_parse_exec(path);
	if (!exec)
		return -1;

	so_start_exec(exec, argv);

	return -1;
}
