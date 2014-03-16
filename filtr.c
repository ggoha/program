#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

//Ошибки main
#define ERARGUMENTS -1

//Ошибки filtr
#define SUCCESS 0
#define ERFORK -1
#define ERPIPE -2
#define ERWAIT -3

const int MAXSIZE = 1000;

int filtr(const char* command)
{
	//Форкаемся и открываем pipe на чтение из сына
	FILE* f = popen(command, "r");
	if (f==NULL)
	{
		perror("Error of new process: ");
		return ERFORK;
	}
	
	char buff[MAXSIZE];
	int count_read, count_new_line=0;
	while ((count_read=fread(buff, sizeof(char), MAXSIZE, f))!=0)
	{
		write(1, buff, count_read);
		int i=0;
		for (; i<count_read; ++i)
			if (buff[i]=='\n')
				++count_new_line;
	}
	
	//Закрываем Pipe
	int pclos = pclose(f);
	//Выводим колличество строк
	write(1, buff, sprintf(buff, "%d\n", count_new_line));
	
	if (pclos<0)
	{
		perror("Error of clouse process: ");
		return ERWAIT;
	}
	return SUCCESS;
}

int main(int argc, char** argv)
{
	if (argc<2)
	{
		write (2, "Too few arguments\n", sizeof("Too few arguments\n"));
		return ERARGUMENTS;
	}

// Вычисляем длину строки из аргументов передаваемой функции
	int i=1, reslength=0;
	for (; i<argc; ++i)
		reslength += strlen(argv[i])+1;
	char* arguments=malloc(reslength);

//Собираем строку из аргументов	
	i=1;
	for (; i<argc-1; ++i)
	{
		strcat(arguments, argv[i]);
		strcat(arguments, " ");
	}
	strcat(arguments, argv[i]);
	arguments[reslength-1]=0;

	filtr(arguments);
	return 0;
}
