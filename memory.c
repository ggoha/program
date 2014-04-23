#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/times.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <errno.h>

struct timeval tv1, tv2 ,dtv;

struct timezone tz;

void time_start()
{
	gettimeofday(&tv1, &tz);
}

long time_stop()
{
	gettimeofday(&tv2, &tz);
	dtv.tv_sec= tv2.tv_sec -tv1.tv_sec;
	dtv.tv_usec=tv2.tv_usec-tv1.tv_usec;
	if(dtv.tv_usec<0)
	{
		dtv.tv_sec--;
		dtv.tv_usec+=1000000;
	}
	return dtv.tv_sec*1000+dtv.tv_usec/1000;
}


//Выставлен размер пайпа
#define SIZE 65535
struct shmstruct
{
	int buff[SIZE];
	int stackoverflow;
	int count_read, count_write;
	sem_t lock_write, lock_read;
	
};

#define ERUNLINK -1
#define ERSHOPEN -2
#define ERFTRUNCATE -3
#define ERCLOSE -4
#define ERMMAP -5
#define ERSEM -6
#define ERFORK -7
#define ERARGUMENT -8

int main(int argc, char** argv)
{	
	//отсутствие буфферизации у стандартного выхода
	setbuf(stdout, NULL);

	//работа с аргументами
	if (argc>2)
	{
		printf("Too many arguments");
		return ERARGUMENT;
	}
	char* end_ptr;

	unsigned long SIZEBUFF;
	if (argc>1)
	{
		SIZEBUFF = strtoul(argv[1], &end_ptr, 10);
		if (*end_ptr)
		{
			printf("Not correct integer");
			return ERARGUMENT;
		}
		if ((SIZEBUFF==LONG_MAX ||SIZEBUFF==LONG_MIN) && errno == ERANGE)
		{
			printf("Not correct integer");
			return ERARGUMENT;		
		}
	}
	else
		SIZEBUFF = 100000;
	
	int fd;
	struct  shmstruct* ptr;

	//создаем новый дескриптор
	fd = shm_open("test", O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR);
	if (fd<0)
		return ERSHOPEN;
	//изменяем размер	
	if (ftruncate(fd, sizeof(struct shmstruct))<0)
	{
		perror("Error ftruncate");
		return ERFTRUNCATE;
	}
	//разделяемая память
	ptr = mmap(NULL, sizeof(struct shmstruct), PROT_READ|PROT_WRITE,MAP_SHARED, fd, 0);
	if (ptr==MAP_FAILED)
	{
		perror("Error mmap");
		return ERMMAP;
	}
	//закрываем дескриптор
	if (close (fd)<0)
	{
		perror("Error close");
		return ERCLOSE;
	}
	//отлинкуемся если уже такой был создан
	if (shm_unlink("test")<0)
		return ERUNLINK;

	//семафор на запись разрешен
	if (sem_init(&ptr->lock_write, 1, 1)<0)
	{
		perror("Error semafor init");
		return ERSEM;
	}
	
	//семафор на чтение запрещен
	if (sem_init(&ptr->lock_read, 1, 0)<0)
	{
		perror("Error semafor init");
		return ERSEM;
	}
	
	srand(time(NULL));	
	
	int pid=fork();
	if (pid<0)
	{
		perror("Error fork");
		return ERFORK;
	}	
	if (pid==0)
	{
		//ребенок читает
		int res=0;
		time_start();	
		int j=0;
		for (; j<SIZEBUFF; ++j)
		{
			sem_wait(&ptr->lock_read);
			if (ptr->count_read==ptr->count_write && !ptr->stackoverflow)
			{
				sem_post(&ptr->lock_write);
				sem_wait(&ptr->lock_read);
			}
			res+=ptr->buff[ptr->count_read];
			ptr->stackoverflow=(ptr->count_write+1-ptr->count_read)==SIZE;
			ptr->count_read=(++(ptr->count_read))%SIZE;
			sem_post(&ptr->lock_read);
		}
		printf("Time: %ld ms\n", time_stop());
		printf("%d ", res);
		return 0;
	}	
	else
	{
		//родитель записывает
		int res_real= 0;
		int j=0;
		for (; j<SIZEBUFF; ++j)
		{
			sem_wait(&ptr->lock_write);
			if (ptr->count_read==ptr->count_write && ptr->stackoverflow)
			{
				sem_post(&ptr->lock_read);
				sem_wait(&ptr->lock_write);
			}
			ptr->buff[ptr->count_write]=rand()%10;
			res_real+=ptr->buff[ptr->count_write];
			ptr->stackoverflow=(ptr->count_write+1-ptr->count_read)==SIZE;			
			ptr->count_write=(++(ptr->count_write))%SIZE;
			sem_post(&ptr->lock_write);
		}
	sem_post(&ptr->lock_read);	
	printf("%d ", res_real);
	return 0;
	}
}
