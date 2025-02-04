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

const char	*get_parasite_opcode( const Elf64_Phdr *phdr )
{
	FILE	*opcode_file = fopen("./src/asm/opcode", "r");
	if (opcode_file == NULL)
		return ((const char *)ptr_perror("\nwoody: failed to open './src/asm/opcode'\n", errno));
	fseek(opcode_file, 0, SEEK_END);
	
	long	file_len = ftell(opcode_file);
	if (file_len == -1)
		return ((const char *)ptr_perror("\nwoody: failed to read the file './src/asm/opcode'\n", errno));
	if (cave_size_ok(phdr, file_len) == 0)
		return ((const char *)ptr_perror("cave is too small. Continuing\n", 0));
	rewind(opcode_file);
	
	char	*buf = calloc(1, file_len);
	if (buf == NULL)
		return ((const char *)ptr_perror("woody: allocation failed", errno));
	if (fread(buf, file_len, 1, opcode_file) == 0)
		return ((const char *)ptr_perror("woody: failed to read the file './src/asm/opcode'\n", 0));
	fclose(opcode_file);
	return (buf);
}

const Elf64_Phdr	*find_cave( const Elf64_Ehdr *elf_h, const Elf64_Phdr *phdr )
{
	// const Elf64_Phdr	*prev = NULL;
	
	write(STDOUT_FILENO, "Looking for a cave ... ", 23);
	for (size_t i = 0; i < elf_h->e_phnum; i++)
	{
		// prev = phdr;
		// printf("p_memz - filesz == %d\n", phdr->p_memsz > phdr->p_filesz);
		if (phdr->p_type == PT_LOAD && phdr->p_memsz > phdr->p_filesz)
		{
			// printf("elf_h->e_phnum == %d\n", elf_h->e_phnum);
			if (phdr->p_memsz > phdr->p_filesz)
			{
				write(STDOUT_FILENO, "Valid cave found!\nChecking cave's size ... ", 44);
				const char *parasite_buf = get_parasite_opcode(phdr);
				if (parasite_buf == NULL)
					goto not_found;
				
				free((void *)parasite_buf);
				return (phdr);
			}
			else
			{
				write(STDOUT_FILENO, "Not enough space: Continuing.\n", 31);
				continue ;
			}
		}
		// prev = phdr;
		phdr++;
	}
	not_found:
		write(STDOUT_FILENO, "No valid cave found ...\nQuitting\n", 33);
		return (NULL);
}

void	inject_code( const void *ptr )
{
	const Elf64_Ehdr	*ehdr = ptr;
	// const Elf64_Addr	original_entry = ehdr->e_entry;
	const Elf64_Phdr	*phdr = (void *)ptr + ehdr->e_phoff;

	if (find_cave(ehdr, phdr) == NULL)
		return ;
}

int	main( int argc, char **argv )
{
	if (argc != 2)
		exit_error("Usage error: invalid number of arguments", 0);
	printf("Opening '%s' ... ", argv[1]);

	int	fd = open(argv[1], O_RDWR);
	if (fd == -1)
		exit_error("Failed", errno);
	printf(" OK\n");

	struct stat	f_stat;
	if (fstat(fd, &f_stat) == -1)
		exit_error("fstat: failed", errno);

	void	*ptr = mmap(NULL, f_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (ptr == MAP_FAILED)
		exit_error("mmap: failed", errno);

	printf("Checking format ... ");
	fflush(stdout);
	if (is_elf64((const char *)ptr, f_stat.st_size) == 0)
		exit_error("\nFormat error: file must be ELF64", 0);
	printf("OK\n");

	inject_code(ptr);
	
	munmap(ptr, f_stat.st_size);
	close(fd);
	return (0);
}