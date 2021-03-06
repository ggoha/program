#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define SUCCESS 0
#define EROPEN -1
#define ERCLOSE -2
#define ERREAD -3

char* GetFullPatch(char* FullName, char* DirectoryName, struct dirent* Dirent) 
{
		
	strcpy(FullName, DirectoryName);
	strcat(FullName, Dirent->d_name);		
	strcat(FullName,"\0");
	return FullName;
}

void WriteRight(struct stat inf, int FlagOfDirectory)
{
	char right[11] = "----------\0";
/*
	if (FlagOfDirectory) 
		right[0] = 'd';

	if ( inf.st_mode & S_IRUSR ) 
		right[1] = 'r';    
	if ( inf.st_mode & S_IWUSR ) 
		right[2] = 'w';
	if ( inf.st_mode & S_IXUSR ) 
		right[3] = 'x';

	if ( inf.st_mode & S_IRGRP ) 
		right[4] = 'r';    
	if ( inf.st_mode & S_IWGRP ) 
		right[5] = 'w';
	if ( inf.st_mode & S_IXGRP ) 
		right[6] = 'x';

	if ( inf.st_mode & S_IROTH ) 
		right[7] = 'r';    
	if ( inf.st_mode & S_IWOTH ) 
		right[8] = 'w';
	if ( inf.st_mode & S_IXOTH ) 
		right[9] = 'x';
*/
	char buff[6];
	sprintf(buff, "%6o", inf.st_mode);

	if (buff[0]==' ' && buff[1]=='4')
		right[0]='d';
	int i= 3;
	for (;i<6; ++i)
	{
		int res = buff[i]-'0';
		if (res%2==1)
			right[(i-3)*3+3]='x';
		res/=2;
		if (res%2==1)
			right[(i-3)*3+2]='w';
		res/=2;
		if (res%2==1)
			right[(i-3)*3+1]='r';
	}
	printf("%s ", right);
}


void WriteInformation(char* NameOfDirectory, struct dirent* Dirent, int FlagOfDirectory)
{
	struct stat inf;
	char* FullPath;
	int size = (strlen(NameOfDirectory) + strlen(Dirent->d_name) + 1) * sizeof(char);
	FullPath = malloc(size);
	GetFullPatch(FullPath, NameOfDirectory, Dirent);
	stat(FullPath, &inf);
	free(FullPath);

	WriteRight(inf, FlagOfDirectory);	
	//Вывод количества hard-ссылок на объект 
	printf("%d ", inf.st_nlink);
	//Вывод пользователя
	if (getpwuid(inf.st_uid)->pw_name==NULL)
		printf("%d ", inf.st_uid);
	else
		printf("%s ", getpwuid(inf.st_uid)->pw_name);
	//Вывод группы
	if (getgrgid(inf.st_uid)->gr_name==NULL)
		printf("%d ", inf.st_uid);
	else
		printf("%s ", getgrgid(inf.st_uid)->gr_name);
	//Вывод размера
	printf("%ld ", inf.st_size);
	//Вывод времени
	char* data = (char*)asctime(localtime(&inf.st_mtime));
	data[strlen(data)-1] = '\0';
	printf("%s ", data);
}


int CountDirectory(char* DirectoryName)
//Подсчет колличества объектов в директории (total)
{
  	DIR* directory = opendir(DirectoryName);
	if (directory==NULL)
	{
		perror("Directory not be open: ");
		return EROPEN;
	}
  	struct dirent* Dirent;	
  	Dirent = readdir(directory);
	int res = 0;
  	while (Dirent != NULL)
  	{
		++res;
		Dirent = readdir(directory);
	}
	int closd = closedir(directory);
	if (closd<0)
	{
		perror("Directory not be closed: ");
		return ERCLOSE;
	}
	return res;
}


int ShowDirectory(char* DirectoryName, int flag[2])
{
	printf("%s\n", DirectoryName);
	DIR* directory = opendir(DirectoryName);
	if (directory==NULL)
	{
		perror("Directory not be open: ");
		return EROPEN;
	}
  	struct dirent* Dirent;

	if (flag[0]) 
		printf("Total: %d\n",CountDirectory(DirectoryName)-2);
	
  	Dirent = readdir(directory);
	while (Dirent != NULL)
  	{
		if (Dirent->d_name[0] != '.') 
		{
			if (flag[0])
				WriteInformation(DirectoryName, Dirent, Dirent->d_type & DT_DIR);						
			printf ("	%s\n", Dirent->d_name);
		}
		Dirent = readdir(directory);
  	}
	printf("\n");
	if (errno!=0)
	{
		perror("Directory not be read: ");
		return ERREAD;			
	}
	int closd = closedir(directory);
	if (closd!=0)
		{
		perror("Directory not be closed");
		return ERCLOSE;
		};
	return SUCCESS;
}

int ls(char* DirectoryName, int flag[2])
{
	ShowDirectory(DirectoryName, flag);
	if (flag[1])
	{	
		DIR* directory = opendir(DirectoryName);
		if (directory==NULL)
		{
			perror("Directory not be open");
			return EROPEN;
		}
  		struct dirent* Dirent;
  		Dirent = readdir(directory);
  		while (Dirent != NULL)
		{
			if (Dirent->d_name[0] != '.') //проверка на скрытость директории 
			{
				if (Dirent->d_type & DT_DIR) 
				{	
					int size = (strlen(DirectoryName)+strlen(Dirent->d_name)+2)*sizeof(char);							
					char* NewDir = malloc(size); 
					strcpy(NewDir, DirectoryName);
					strcat(NewDir, Dirent->d_name);
					strcat(NewDir, "/");
					ls(NewDir, flag);
					free(NewDir); 
				}
			}
			Dirent = readdir(directory);
  		
		}
		if (errno!=0)
		{
			perror("Directory not be read: ");
			return ERREAD;			
		}
		int closd = closedir(directory);
		if (closd!=0)
			{
			perror("Directory not be closed: ");
			return ERCLOSE;
			}
	}
	return SUCCESS;
}

int main (int argc, char *argv[])
{
	
	if (argc > 3) 
	{
		write(1, "Too much arguments\n", sizeof("Too much arguments\n"));
	}
	else
	{
		int i=0, flag[2] = {0, 0};		
		for (i=1; i<argc; ++i)
		{
			if (!strcmp(argv[i], "-l")) 
				flag[0] = 1;
			if (!strcmp(argv[i], "-R")) 
				flag[1] = 1;
		}
		ls("./", flag);			
	}	
	return 0;
}
