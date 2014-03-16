#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>

#define SUCCESS 0

//ER_MAIN
#define ERARGUMENTS -1

//ER_FIND	
#define ERSTATUS -1
#define ERRECURS -2
#define ERTYPES -3
//ERFILE
#define EROPEN -1
#define ERCREATE -2
#define ERREAD -3
#define ERWRITE -4
//ERDIR
#define EROPENDIR -1
#define ERCLOSED -2

void PrintError (char* error, char* path)
{
	char* res=malloc(strlen(error));
	strcpy(res, error);
	strcat(res, path);
	perror(res);
	free(res);
}

//Извлекаем все -R если они есть
int ProseccingInputDate(int* argc, char** argv)
{
	int i=1, count=0, recursive=0;	
	for (;i<*argc; ++i)
	{
		if (!strcmp(argv[i], "-R"))
			{
				++count;
				recursive=1;
			} 
	}
	int ch =1;	
	for (;ch<=count; ++ch)
	{
		i=1;		
		while (strcmp(argv[i], "-R"))
			++i;
		int j=i;		
		for (;j<*argc-1;++j)
			argv[j]=argv[j+1];	
	}
	*argc-=count;
	return recursive;
}


int* prefix(char* s)
 {
    int* pref = malloc(strlen(s));
    int i=1, k=0;
    for (;i<strlen(s); ++i)
    {
        while (k > 0 && s[k] != s[i])
            k = pref[k-1];
        if (s[k] == s[i])
            ++k;
        pref[i] = k;
	}
    return pref;
 }

	
char* kmp(char* s, int* pref, char* t)
{
    char* index = NULL;
    int i=0, k=0;
    for (;i<(strlen(t));++i)
    {
        while (k > 0 && s[k] != t[i])
            k = pref[k-1];
        if (s[k] == t[i])
            k++;
        if (k == strlen(s))
        {
            index = t+(i - strlen(s) + 1);
            break;
		}
	}
    return index;
}	

int max (int a, int b)
{
	if (a>b) return a; else return b;
}

int FindInFile(char* str, int* pref,  char* path)
{
	int fd_open=open(path, O_RDONLY|O_APPEND);
	if (fd_open<0)
	{
		PrintError("Cann't open ", path);
		return EROPEN;
	}
	int SIZE=strlen(str)*100;
	char buff[SIZE+strlen(str)];
	int j=0;
	for (;j<SIZE+strlen(str);++j)
		buff[j]=0;
	char* buffnew=buff+strlen(str);
	int count_read;
	//так как буфер конечной длины, то буферы накладываем друг на друга, чтобы поиск увенчался успехом
	while((count_read = read (fd_open, buff+strlen(str), SIZE)) != 0)
	{
//		printf("%s\n!", buffnew);
		char* res;
		//поиск
		while ((res=kmp(str, pref, buffnew))!=NULL)
		{
			printf("%s: %s\n", path, res);
			buffnew=res+strlen(str);
		}
		//перекладывание пересечения буфера
		if (count_read>strlen(str))
		{
			int j=1;
			for (;j<strlen(str);++j)
				buff[j]=buff[SIZE+j];
			for (;j<SIZE+strlen(str);++j)
				buff[j]=0;

			buffnew=buff+1;
		}
		else
		{
			int j=1;
			for (;j<strlen(str);++j)
				buff[j]=buff[strlen(str)+j];
			for (; j<SIZE+strlen(str); ++j)
				buff[j]=0;
			buffnew=buff+strlen(str)-count_read+1;			
		}
		
	}
	return SUCCESS;
}

int FindInDirectoryRecursive(char* str, int* pref,  char* path)
{
	DIR* directory = opendir(path);
	if (directory==NULL)
	{
		PrintError("Cann't open ", path);
		return EROPEN;
	}
  	struct dirent* Dirent;
  	Dirent = readdir(directory);
  	while (Dirent != NULL)
	{
		if (Dirent->d_name[0] != '.') 
		{
			if (Dirent->d_type & DT_DIR) 
			{	
				int size = (strlen(path)+strlen(Dirent->d_name)+2)*sizeof(char);							
				char* NewDir = malloc(size); 
				strcpy(NewDir, path);
				strcat(NewDir, Dirent->d_name);
				strcat(NewDir, "/");
				FindInDirectoryRecursive(str, pref, NewDir);
				free(NewDir); 
			}
			else
			{
				int size = (strlen(path)+strlen(Dirent->d_name)+1)*sizeof(char);							
				char* NewDir = malloc(size); 
				strcpy(NewDir, path);
				strcat(NewDir, Dirent->d_name);
				FindInFile(str, pref, NewDir);
				free(NewDir); 
			}
		}
		Dirent = readdir(directory);
	}
	if (errno!=0)
	{
		PrintError("Cann't read ", path);
		return ERREAD;			
	}
	int closd = closedir(directory);
	if (closd!=0)
	{
		PrintError("Cann't closed ", path);
		return ERCLOSED;
	}
	return SUCCESS;
}

int find(char* str, int* pref,  char* path, int recursive)
{
	struct stat stat_path;
	if (stat(path, &stat_path)) 
	{	
		PrintError("Cann't get status ", path);
		return ERSTATUS;
	}
	if (stat_path.st_mode & S_IFDIR) 
	{
		if (recursive)
		{	
			return FindInDirectoryRecursive(str, pref, path);
		}
		else 
		{
			write(2, "Cann't find non-recursive \n", sizeof("Cann't find non-recursive \n"));
			return ERRECURS;
		}
	}
	else
	{
		return FindInFile (str, pref, path);
	}
	
}	

int main (int argc, char *argv[])
{
	int flagR = ProseccingInputDate(&argc, argv);
	if (argc<3)
	{
			write(2, "Too fee arguments\n", sizeof("Too fee arguments\n"));
			return ERARGUMENTS;		
	}
	else
	{
		int i=2;	
		int* f = prefix(argv[1]);
		for (;i<argc;++i)
			find(argv[1], f, argv[i], flagR);	
	}	
	return 0;
}
