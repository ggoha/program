#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

#define ERARGUMENTS -4
#define ERLIMIT -3
#define ERFORK -2
#define EREXEC -1
#define SUCSESS 0

//Коэфициент с которым программа спит/работает
const int k=100;

//Обработчик смерти сына
void handler(int sign) 
{
		pid_t pid;
		int stat;
		pid = waitpid(-1, &stat, WNOHANG);
		if (pid>0)
			exit(SUCSESS);			
}

int cpulimit(int argc, char** argv, int lim)
{

	int pid=fork();
    switch (pid)
    {
	case 0: 
    {

        if (execv(argv[2], argv+3)<0)
		{
			perror("exec one failed: ");
			exit(EREXEC);
		}
        exit(SUCSESS);
    }
    case -1:
	{
		perror("fork error: ");
		exit (ERFORK);
	}
	default:
	{
		for (;;)
		{
			kill(pid, SIGSTOP);
			while (usleep((100-lim)*k)!=0)
				if (errno != EINTR)
					perror("usleep error: ");
			kill(pid, SIGCONT);
			while (usleep((lim)*k)!=0)
				if (errno != EINTR)
					perror("usleep error: ");
//			write (1, "!", 1);
		}
	}
	}	
}

int main(int argc, char** argv)
 {
	//устанавливаем свою реакцию на сигналл
	if (signal(SIGCHLD, handler)!=0)
		perror("signal SIGCHLD error: ");
	
	//получаем коэфициент загрузки
	char** endptr=NULL;
	unsigned long lim = strtoul(argv[1], endptr, 10);
	if (endptr!=NULL)
	{
		write(2, "Eror limit\n", sizeof("Eror limit\n"));
		return ERLIMIT;
	}
	
	if (argc<3)
	{
		write(2, "Too few argaments\n", sizeof("Too few argaments\n"));
		return ERARGUMENTS;
	}
	
	cpulimit(argc, argv, lim);
	return 0;
}
