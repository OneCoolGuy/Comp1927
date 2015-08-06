#include <stdlib.h>
#include <stdio.h>



void seq(int start, int finish, int step);
void checkSeq(int start, int finish, int step);

int main(int argc, char **argv)
{
	int step, start, finish;
	
	
	switch (argc){
	case 2: 
		start = 1;
		finish = atoi(argv[1]);
		step = (start < finish) ? 1:-1;
		break;
	case 3:
		start = atoi(argv[1]);
		finish = atoi(argv[2]);
		step = (start < finish) ? 1:-1;
		break;
	case 4:
		start = atoi(argv[1]);
		finish = atoi(argv[2]);
		step = atoi(argv[3]);
		checkSeq(start, finish, step);
		break;
	default:
		fprintf(stderr, "Usage: %s [start] [step] finish\n", argv[0]);
		exit(EXIT_FAILURE);
		break;
	}

	seq(start, finish, step);

	return EXIT_SUCCESS;
}


void seq(int start, int finish, int step)
{
	int i;
	for(i = start; i < finish; i += step){
		printf("%d ", i);
	}
	printf("\n");
}

void checkSeq(int start, int finish, int step)
{
	int ok = 1;
	if (step == 0) {
		ok = 0;
	}
	else if (start < finish && step < 0) {
		ok = 0;
	}
	else if (start > finish && step > 0) {
		ok = 0;
	}
	if (!ok) {
		fprintf(stderr, "Invalid step %d\n", step);
		exit(EXIT_FAILURE);
	}
}
