#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <sys/time.h>
#include <time.h>

#define ERPIPE -1
#define SUCSESS 0
const long long SIZE = 32000;

struct timeval tv1,tv2,dtv;

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

long long min(long long a, long long b)
{
	if (a<b) return a;
	else return b;
}

int speedtest (void)
{
	//создание Pipe
	int descriptors[2];
	if (pipe(descriptors))
	{
		perror("Pipe_error");
		return ERPIPE;
	}
	//форкаемся 	
	int processid = fork();
	switch (processid)
	{
		case 0:
		{
			//Закрываем себе чтение
			close(descriptors[0]);
			//Создаем буфер
			char* buff=malloc(SIZE);
			int k=0;
			for (;k<SIZE; ++k)
				buff[k]='0';
			char* buff_new=buff;
			long long sizebuff = 1000000000;
			long long j=0;
			//начало замера времени
			time_start();			
			
			//запись в pipe
			while (j<sizebuff)
			{
				long long size = min(SIZE,strlen(buff_new));
				long long count_w;
				while (j<sizebuff && (count_w=write(descriptors[1], buff_new, size))!=0)
				{	
//					printf("%Lf\n", (long double)(j)/sizebuff);
					buff_new+=count_w;
					size-=count_w;
					j+=count_w;
				}
				j+=count_w;
				buff_new=buff;
			}
			
			printf("Time: %ld ms\n", time_stop());			
			//Закрываемся
			close(descriptors[1]);
			exit(SUCSESS);
		}
		case -1:
		{
				perror("Error fork");
		}
		default:
		{
			close(descriptors[1]);
			//Читаем результат из труб
			char* buff = malloc(SIZE);
			int count_r;
			while ((count_r=read(descriptors[0], buff, SIZE))!=0)
//				write(1, buff, count_r);
				;
			close(descriptors[0]);
			return SUCSESS;
		}
	}
}

int main(int argc, char **argv)
{
	speedtest ();
	return 0;
}

