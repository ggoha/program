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

//ER_COPY	
#define ERSTATUS -1
#define ERRECURS -2
#define ERTYPES -3
//ERFILEFILE
#define EROPEN -1
#define ERCREATE -2
#define ERREAD -3
#define ERWRITE -4
//ERDIRDIR
#define EROPENDIR -1

void PrintError (char* error, char* path)
{
	char* res=malloc(strlen(error));
	strcpy(res, error);
	strcat(res, path);
	perror(res);
	free(res);
}


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

int Copy(char*, char*, int);

int CopyDirDirRecursive(char* from, char* where)
{
	DIR* dir_from = opendir(from);
	if (dir_from == NULL) 
	{
		PrintError("Cann't open directory", from);
		return EROPENDIR;
	}
	struct stat stat_from, stat_where;
	if (stat(from, &stat_from)) 
	{	
		PrintError("Cann't get status ", from);
		return ERSTATUS;
	}
	
	char* wherenew;
	if (mkdir(where, stat_from.st_mode))
	{
		char* name = strrchr(from, '/')	;
		if (name==NULL)
			name=from;
		else name=name+1;
		wherenew = malloc((strlen(where)+strlen(name)+2)*sizeof(char));
		strcpy(wherenew, where);
		strcat(wherenew, "/");
		strcat(wherenew, name);
		if (mkdir(wherenew, stat_from.st_mode) && errno != EEXIST) 
		{
			PrintError("Cann't open direectory ", wherenew);
			free(wherenew);
			closedir(dir_from);
			return EROPEN;
		}			
	}
	else
	{
		wherenew = malloc(strlen(where));
		strcpy(wherenew, where);		
	}
  	struct dirent* Dirent;
  	while ((Dirent = readdir(dir_from)) != NULL)
	{
		if (Dirent->d_name[0] != '.')
		{ 
			int size = (strlen(from)+strlen(Dirent->d_name)+2)*sizeof(char);							
			char* from_new = malloc(size); 
			strcpy(from_new, from);
			strcat(from_new, "/");
			strcat(from_new, Dirent->d_name);
			Copy(from_new, wherenew, 1);
			free(from_new); 
		}
	}
	//return 0;
}

int CopyFileFile(char* from, char* where)
{
	int fd_from=open(from, O_RDONLY|O_APPEND);
	if (fd_from<0)
	{
		PrintError("Cann't open ", from);
		return EROPEN;
	}
	struct stat stat_from;
	stat(from, &stat_from);
	int fd_where=open(where, O_WRONLY|O_TRUNC|O_CREAT, stat_from.st_mode);
	if (fd_where<0)
	{
		PrintError("Cann't open ", where);
		return EROPEN;
	}
	char buff[1000];
	while (1)
	{
		int count_read = read (fd_from, buff, 1000);
		if (count_read<0)
		{
			PrintError("Cann't read ", from);
			return ERREAD;
		}
		else 
			if (count_read!=0)
			{
				int count_write = write (fd_where, buff, count_read);
				if (count_write<0)
				{
					PrintError("Cann't write ", where);
					return ERWRITE;
				}			
			}
			else return SUCCESS;
	}	
	
}

int CopyFileDir(char* from, char* where)
{
	char* name = strrchr(from, '/')	;
	if (name==NULL)
		name=from;
	else name=name+1;
	char*  wherenew = malloc(strlen(where));
	strcpy(wherenew, where);
	strcat(wherenew, "/");
	strcat(wherenew, name);
	int result = CopyFileFile(from, wherenew);
	free(wherenew);
	return result;
}


int Copy(char* from, char* where, int flagR)
{
//	fprintf(stderr, "Copy: f: %s w: %s\n", from, where);
//	printf("%s %s\n", from, where);
	struct stat stat_from, stat_where;
	if (stat(from, &stat_from)) 
	{	
		PrintError("Cann't get status ", from);
		return ERSTATUS;
	}
	if (stat_from.st_mode & S_IFDIR) 
	{
		if (flagR)
			if (!stat(where, &stat_where) && !(stat_where.st_mode & S_IFDIR))
			{	
				PrintError("Cann't get write into file directory", from);
				return ERTYPES;
			}
			else
				return CopyDirDirRecursive(from, where);
		else 
		{
			write(2, "Cann't copy non-recursive directory \n", 37);
			return ERRECURS;
		}
	}
	else
	{
		if (stat(where, &stat_where)) 
			return CopyFileFile(from, where);
		else if (stat_where.st_mode & S_IFDIR)
			return CopyFileDir(from, where);
			else return CopyFileFile (from, where);
	}
}

int main(int argc, char** argv)
{
	int flagR = ProseccingInputDate(&argc, argv);
	if (argc<3)
		{
			write(2, "Too fee arguments\n", 19);
			return ERARGUMENTS;		
		}

	int i=1;	
	for (;i<argc-1;++i)
		Copy(argv[i], argv[argc-1], flagR);	
	return 0;
}
