#include <stdio.h>
#include <stdlib.h>


int main()
{
	unsigned int i = 256;
	char * c = (char *)&i;
	printf("%x\n", c);
	printf("%i\n", c);
	printf("%x\n", i);
	printf("%i\n", i);
	return 0;
}

	
