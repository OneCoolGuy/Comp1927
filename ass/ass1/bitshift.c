#include <stdio.h>
#include <stdlib.h>


int main()
{
	unsigned int i = 512;
	i = i >> 8;
	char * c = (char *)&i;

	printf("%c\n", *c);
	printf("%i\n", *c);
	printf("%x\n", i);
	printf("%i\n", i);
	return 0;
}

	
