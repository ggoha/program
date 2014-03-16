#include <unistd.h>
int main(int argc, char** argv)
{
	int i=1;
	for (;i<1000000;++i)
		write (1, "@", 1);
	return 0;
}

