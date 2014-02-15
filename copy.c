#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

int main(int argc, char** argv)
{
	if (argc!=3)
		if (argc<3)
			write (2, "A few arguments\n", 16);
		else 
			write (2, "A many arguments\n", 17);
	else
	{
		int fd_1=open(argv[1], O_RDONLY|O_APPEND);
		if (fd_1<0)
		{
			perror("File1 not be open");
			_exit(1);
		}
		int fd_2=open(argv[2], O_WRONLY|O_TRUNC|O_APPEND);
		if (fd_2<0)
		{
			fd_2=open(argv[2], O_CREAT|O_WRONLY|O_APPEND);
			if (fd_2<0)
				{
				perror("File2 not be create");
				_exit(1);
				}
		}
		char buff[1000];
		while (1)
		{
 			int count_r = read (fd_1, buff, 1000);
			if (count_r<0)
			{
				perror("File1 not be read");
				_exit(2);			
			}
			else 
				if (count_r!=0)
				{
					int count_w = write (fd_2, buff, count_r);
				}
				else return 0;
		}	
	}	
	return 0;
} 	
