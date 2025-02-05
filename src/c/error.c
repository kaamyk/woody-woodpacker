#include "../../inc/woody.h"

void	*ptr_perror( const char *err_mess, int err_code )
{
	if (err_code == 0)
		fprintf(stderr, "%s\n", err_mess);
	else
		fprintf(stderr, "%s: %s\n", err_mess, strerror(err_code));
	return (NULL);
}

bool	bool_perror( const char *err_mess, int err_code )
{
	if (err_code == 0)
		fprintf(stderr, "%s\n", err_mess);
	else
		fprintf(stderr, "%s: %s\n", err_mess, strerror(err_code));
	return (1);
}
