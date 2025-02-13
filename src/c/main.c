#include "../../inc/woody.h"

void	exit_error( const char *err_str, const int err_code )
{
	if (errno != 0)
	{
		fprintf(stderr, "%s: %s\n", err_str, strerror(err_code));
		exit (2);
	}
	else
	{
		fprintf(stderr, "%s\n", err_str);
		exit (2);
	}
	exit(2);
}

void	print_hexa( const uint8_t *p, const size_t len )
{
	printf("Printing memory area from %p to %p ... \n", p, p+len);
	for (size_t i = 0; i < len; i++)
	{
		if (i != 0 && i % 8 == 0)
			printf("\n");
		else if (i != 0 && i % 4 == 0)
			printf("   ");
		printf("%.2x ", p[i]);
	}
	printf("\n");
}

bool	is_elf64( const char *ptr, const off_t size )
{
	return (ptr[EI_MAG1] == 'E' && ptr[EI_MAG2] == 'L'
		&& ptr[EI_MAG3] == 'F' && ptr[EI_CLASS] == ELFCLASS64
		&& (unsigned long) size > sizeof(Elf64_Ehdr));
}

bool	cave_size_ok(const Elf64_Phdr *phdr, const long file_len )
{
	return ((unsigned long)file_len <= phdr->p_memsz + phdr->p_offset);
}

const uint8_t	*get_parasite_opcode( uint8_t **opcode_buf, long *opcode_len )
{
	write(STDOUT_FILENO, "Getting parasite code ... ", 26);
	FILE	*opcode_file = fopen("./src/asm/opcode", "r");
	if (opcode_file == NULL)
		return ((const uint8_t *)ptr_perror("\nwoody: failed to open './src/asm/opcode'\n", errno));
	fseek(opcode_file, 0, SEEK_END);
	
	*opcode_len = ftell(opcode_file);
	if (*opcode_len == -1)
	{
		fclose(opcode_file);
		return ((const uint8_t *)ptr_perror("\nwoody: failed to read the file './src/asm/opcode'\n", errno));
	}
	write(STDOUT_FILENO, "OK\n", 3);
	rewind(opcode_file);
	
	*opcode_buf = calloc(1, *opcode_len);
	if (*opcode_buf == NULL)
	{
		fclose(opcode_file);
		return ((const uint8_t *)ptr_perror("woody: allocation failed", errno));
	}
	if (fread(*opcode_buf, *opcode_len, sizeof(uint8_t), opcode_file) == 0)
	{
		fclose(opcode_file);
		return ((const uint8_t *)ptr_perror("woody: failed to read the file './src/asm/opcode'\n", 0));
	}
	fclose(opcode_file);
	printf("opcode len == %ld\n", *opcode_len);
	return (*opcode_buf);
}

Elf64_Phdr *find_cave_segment( Elf64_Ehdr *elf_h, Elf64_Phdr *phdr, const long opcode_len )
{
	uint8_t	load_nb = 0;
	write(STDOUT_FILENO, "Looking for a cave ... ", 23);
	for (size_t i = 0; i < elf_h->e_phnum; i++, phdr++)
	{
		if (phdr->p_type == PT_LOAD && (phdr + 1)->p_type == PT_LOAD && ++load_nb && (phdr->p_flags & PF_X) && phdr->p_filesz > 0 && phdr->p_memsz == phdr->p_filesz)
		{
			printf("Cave found at %p!\nChecking cave's size ... (segment %ld | LOAD %hhu)", 
				(void *)(phdr->p_vaddr + phdr->p_filesz), i, load_nb);
			if (cave_size_ok(phdr, opcode_len) == 0)
				write(STDOUT_FILENO, "Too small. Continuing\n", 23);
			write(STDOUT_FILENO, "Cave is valid\n", 15);
			return (phdr);
		}
		
	}
	write(STDOUT_FILENO, "No valid cave found ...\nQuitting\n", 33);
	return (NULL);
}

Elf64_Shdr	*find_cave_section( Elf64_Ehdr *ehdr, Elf64_Shdr *shdr )
{
	write(STDOUT_FILENO, "Looking for the section ... ", 28);
	for (size_t i = 0; i < ehdr->e_shnum; i++, shdr++)
	{
		if (shdr->sh_offset == 0x1168 && (shdr->sh_flags & SHF_EXECINSTR))
		{
			write(STDOUT_FILENO, "OK\n", 3);
			return (shdr);
		}
	}
	return (NULL);
}

void	modify_entrypoint( Elf64_Ehdr *ehdr, Elf64_Phdr *cave_segment )
{
	write(STDOUT_FILENO, "Generating new entry point ... ", 31);
	ehdr->e_entry = (Elf64_Addr)(cave_segment->p_vaddr + cave_segment->p_memsz);
	write(STDOUT_FILENO, "OK\n", 3);
	printf("New entry point is -> 0x%lx\n", ehdr->e_entry);
}

void	modify_lengths( Elf64_Phdr *cave_segment, Elf64_Shdr *cave_section, const long parasite_len )
{
	cave_segment->p_filesz += parasite_len;
	cave_segment->p_memsz += parasite_len;
	cave_section->sh_size += parasite_len;
}

void	modify_parasite_jmp( const Elf64_Addr original_entry, uint8_t *parasite_code, const long parasite_len )
{
	write(STDOUT_FILENO, "Modifying jmp instruction ... ", 30);
	printf("originale_entry == 0x%lx\n", original_entry);
	long	index_jmp = 0;
	
	while (index_jmp < parasite_len && parasite_code[index_jmp] != 0xe9)	
	{
		index_jmp++;
	}
	if (index_jmp >= parasite_len || *(parasite_code + index_jmp) != 0xe9)
	{
		write(STDOUT_FILENO, "jmp instruction not found\n", 27);
		return ;
	}
	printf("original_entry == 0x%lx | jmp opcode == 0x%x | jmp + 1 opcode == 0x%x\n", original_entry, (parasite_code + index_jmp)[0], (parasite_code + index_jmp)[1]);
	int32_t	jmp_offset = original_entry - (0x1175 + index_jmp + 5); 
	memcpy(&parasite_code[index_jmp + 1], &jmp_offset, sizeof(int32_t));
	write(STDOUT_FILENO, "OK\n", 3);
}

void	insert_code( void *cave, uint8_t *parasite_opcode, const long parasite_len )
{
	write(STDOUT_FILENO, "Inserting code in binary ... ", 30);
	fflush(stdout);
	memcpy(cave, parasite_opcode, parasite_len);
	write(STDOUT_FILENO, "OK\n", 3);
}

void	inject_code( void *ptr )
{
	uint8_t	*parasite_opcode = NULL;
	long	parasite_len = 0;
	Elf64_Ehdr	*ehdr = ptr;
	Elf64_Phdr	*phdr = (void *)ptr + ehdr->e_phoff;
	Elf64_Shdr	*shdr = (void *)ptr + ehdr->e_shoff;
	const Elf64_Addr original_entry = ehdr->e_entry;
	(void) original_entry;

	if (get_parasite_opcode(&parasite_opcode, &parasite_len) == NULL || parasite_opcode == NULL)
		return ;
	print_hexa(parasite_opcode, parasite_len);
	phdr = find_cave_segment(ehdr, phdr, parasite_len);
	if (phdr == NULL)
		goto end_injection;
	shdr = find_cave_section(ehdr, shdr);
	if (shdr == NULL)
		goto end_injection;
	modify_entrypoint(ehdr, phdr);
	modify_lengths(phdr, shdr, parasite_len);

	printf("Original entrypoint is %p | New entrypoint is %p\n", (void *)original_entry, (void *)ehdr->e_entry);
	modify_parasite_jmp(original_entry, parasite_opcode, parasite_len);
	print_hexa(parasite_opcode, parasite_len);
	insert_code((void *)(ptr + ehdr->e_entry), parasite_opcode, parasite_len);
	printf("code inserted at offset %lx \n", ehdr->e_entry);
	// insert_code(ptr + cave_segment->p_offset + cave_segment->p_filesz, parasite_opcode, parasite_len);
	printf("cave_segment->p_offset -> 0x%lx | cave_segment->filesz -> 0x%lx (%ld) | cave->segment->memzs -> 0x%lx (%ld) | cave_section->offset -> 0x%lx | cave_section->size -> 0x%lx\n"
		, phdr->p_offset, phdr->p_filesz, phdr->p_filesz, phdr->p_memsz, phdr->p_memsz, shdr->sh_offset, shdr->sh_size);
	
	end_injection:
		free(parasite_opcode);
}

int	create_duplicated_file( void *ptr, long map_size )
{
	write(STDOUT_FILENO, "Creating the duplicate ... ", 27);
	int	fd = open("woody", O_CREAT | O_TRUNC | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
	if (fd == -1)
	{
		(void) bool_perror("KO\nFailed to create duplicate", errno);
		return (fd);
	}
	
	write(STDOUT_FILENO, "OK\nWriting in file ... ", 24);
	if (write(fd, (char *)ptr, map_size) == -1)
	{
		close(fd);
		(void) bool_perror("KO\nFailed to write in duplicate", errno);
		return (-1);
	}
	write(STDOUT_FILENO, "OK\n", 3);
	return (fd);
}


int	main( int argc, char **argv )
{
	if (argc != 2)
		exit_error("Usage error: invalid number of arguments", 0);
	printf("Opening '%s' ... ", argv[1]);
	fflush(stdout);

	int	fd = open(argv[1], O_RDWR);
	if (fd == -1)
		exit_error("Failed", errno);
	write(STDOUT_FILENO, "OK\n", 3);

	struct stat	f_stat = {0};
	if (fstat(fd, &f_stat) == -1)
		exit_error("fstat: failed", errno);

	write(STDOUT_FILENO, "Remapping the file ... ", 24);
	void	*ptr = mmap(NULL, f_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (ptr == MAP_FAILED)
		exit_error("mmap: failed", errno);
	write(STDOUT_FILENO, "OK\n", 3);
	printf("New map 0x%lx bytes at address %p (to %p)\n", f_stat.st_size, ptr, f_stat.st_size + ptr);

	write(STDOUT_FILENO, "Checking format ... ", 20);
	fflush(stdout);
	if (is_elf64((const char *)ptr, f_stat.st_size) == 0)
		exit_error("\nFormat error: file must be ELF64", 0);
	write(STDOUT_FILENO, "OK\n", 3);

	int	dupfd = create_duplicated_file(ptr, f_stat.st_size);
	if (dupfd == -1)
		return (1);
	munmap(ptr, f_stat.st_size);
	close(fd);
	
	if (fstat(dupfd, &f_stat) == -1)
		exit_error("fstat: failed", errno);
	ptr = mmap(NULL, f_stat.st_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, dupfd, 0);
	if (ptr == MAP_FAILED)
		exit_error("mmap: failed", errno);
	inject_code(ptr);
	
	if (msync(ptr, f_stat.st_size, MS_SYNC) == -1)
		exit_error("msync: failed", errno);
	Elf64_Ehdr	*ehdr = ptr;
	Elf64_Phdr	*phdr = ptr + ehdr->e_phoff;
	printf("entrypoint == 0x%lx(%ld) | segment offset == 0x%lx(%ld) | end of segment == 0x%lx(%ld) | segment size == 0x%lx(%ld)\n"
		, ehdr->e_entry,  ehdr->e_entry, phdr[3].p_offset, phdr[3].p_offset, 
		phdr[3].p_offset + phdr[3].p_filesz, phdr[3].p_offset + phdr[3].p_filesz, 
		phdr[3].p_filesz, phdr[3].p_filesz);
	munmap(ptr, f_stat.st_size);
	close(dupfd);
	
	
	dupfd = open("woody", O_RDWR, 0);
	if (dupfd == -1)
		printf(">>> 2nd open failed\n");
	bzero(&f_stat, sizeof((f_stat)));
	if (fstat(dupfd, &f_stat) == -1)
		exit_error("fstat: failed", errno);
	printf("fstat size %ld\n", f_stat.st_size);
	ptr = mmap(NULL, f_stat.st_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, dupfd, 0);
	if (ptr == MAP_FAILED)
		exit_error("mmap: failed", errno);
	ehdr = ptr;
	phdr = (void *)ptr + ehdr->e_phoff;
	printf("entrypoint == 0x%lx(%ld) | segment offset == 0x%lx(%ld) | end of segment == 0x%lx(%ld) | segment size == 0x%lx(%ld)\n"
		, ehdr->e_entry,  ehdr->e_entry, phdr[3].p_offset, phdr[3].p_offset, 
		phdr[3].p_offset + phdr[3].p_filesz, phdr[3].p_offset + phdr[3].p_filesz, 
		phdr[3].p_filesz, phdr[3].p_filesz);
	munmap(ptr, f_stat.st_size);
	close(dupfd);
	return (0);
}