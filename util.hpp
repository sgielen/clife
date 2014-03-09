#include <sys/time.h>

void init_random() {
	struct timeval tp;
	gettimeofday(&tp, NULL);
	srand(tp.tv_usec);
}
