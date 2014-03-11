#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>


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
	printf("%s ", getpwuid(inf.st_uid)->pw_name);
	//Вывод группы
	printf("%s ", getgrgid(inf.st_uid)->gr_name);
	//Вывод размера
	printf("%ld ", inf.st_size);
	//Вывод размера
	char* data = (char*)asctime(localtime(&inf.st_mtime));
	data[strlen(data)-1] = '\0';
	printf("%s ", data);
}


int CountDirectory(char* DirectoryName)
{
  	DIR* directory = opendir(DirectoryName);
	if (directory==NULL)
	{
		perror("Directory not be open");
		return -1;
	}
  	struct dirent* Dirent;	
  	Dirent = readdir(directory);
	int res = 0;
  	while (Dirent != NULL)
  	{
		struct stat inf;
		char* FullPath;
		int size = (strlen(DirectoryName) + strlen(Dirent->d_name) + 1) * sizeof(char);
		FullPath = malloc(size);
		GetFullPatch(FullPath, DirectoryName, Dirent);
		stat(FullPath, &inf);
		free(FullPath);
		res += inf.st_blocks; //?
		Dirent = readdir(directory);

	}
	int closd = closedir(directory);
	return res;
}


int ShowDirectory(char* DirectoryName, int flag[2])
{
	printf("%s\n", DirectoryName);
	DIR* directory = opendir(DirectoryName);
	if (directory==NULL)
	{
		perror("Directory not be open");
		return -1;
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
	printf("\n\n");
	if (errno!=0)
	{
		perror("Directory not be read");
		return -3;			
	}
	int closd = closedir(directory);
	if (closd!=0)
		{
		perror("Directory not be closes");
		return -2;
		};
	return 0;
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
			return -1;
		}
  		struct dirent* Dirent;
  		Dirent = readdir(directory);
  		while (Dirent != NULL)
		{
			if (Dirent->d_name[0] != '.') 
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
			perror("Directory not be read");
			return -3;			
		}
		int closd = closedir(directory);
		if (closd!=0)
			{
			perror("Directory not be closes");
			return -2;
			}
	}
	return 0;
}

int main (int argc, char *argv[])
{
	
	if (argc > 3) 
	{
		write(1, "Too much arguments\n", 19);
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
		write(1, "\n", 1);	
	}	
	return 0;
}
