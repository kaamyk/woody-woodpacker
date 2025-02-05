#include <stdio.h>
#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>

//	ERROR.C
void	*ptr_perror( const char *, const int );
bool	bool_perror( const char *err_mess, int err_code );
