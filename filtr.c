#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

//Ошибки main
#define ERARGUMENTS -1

//Ошибки filtr
#define SUCCESS 0
#define ERFORK -1
#define ERWAIT -3
#define ERPIPE -4
#define EREXEC -5

const int MAXSIZE = 1000;

int filtr(char** argv)
{
	//Форкаемся и открываем pipe на чтение из сына
	int pipe_d[2];
	if (pipe(pipe_d)==-1)
	{
		perror("Error of pipe: ");
		return ERPIPE;
	}
	int processid = fork();
	if (processid==0)
	{
	//Ребенок переназначает вывод
		close(pipe_d[0]);
		dup2 (pipe_d[1], 1);
		if (execv(argv[1], argv+1)==-1)
		{
			perror("Error of exec: ");
			return EREXEC;
		}
		close (pipe_d[1]);
		exit(SUCCESS);
	}
	else if (processid==-1)
		{
			perror("Error of fork: ");
			return ERPIPE;		
		}
		else
		{
			//Родитель читает выводит считает
			close(pipe_d[1]);
			char buff [MAXSIZE];
			int count_new_line=0, read_count; 
			while ((read_count=read(pipe_d[0], buff, MAXSIZE))!=0)
			{
					write (1, buff, read_count);
					int i=0;
					while (i<read_count)
						if (buff[i++]=='\n')
							++count_new_line;
					write (1, "!", 1);
			}
			//Выводим колличество строк
			write(1, buff, sprintf(buff, "%d\n", count_new_line));
		}
	return SUCCESS;
}

int main(int argc, char** argv)
{
	filtr(argv);
	return SUCCESS;
}
