#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>
#include <unistd.h>
#include <string.h>

#define ERTHREAD -3
#define ERARGUMENTS -1
#define ERMUTEX -2
int start=0;

struct fork
{
		int id;
		pthread_mutex_t mutex;
};

struct philosother
{
		int id, count_philosopher;
		struct fork* left_fork, *right_fork;
		pthread_t thread;
};

struct ThreadParam 
{
	struct philosother* p;
	int n;
};

void* philosothers_live(struct philosother* philosother_)
{
	int global_eat_time=0, global_think_time=0, global_wait_time=0;
	char buff[100];
	while (start==0);
	while (start!=0)
	{
		//Спим
		int time_thinking = random()%10+1;
		write (1, buff, sprintf(buff, "Philosopher %d think at %d second\n", philosother_->id, time_thinking));
		global_think_time+=time_thinking;
		sleep(time_thinking);
		//Берем вилки
		int flag=0;
		int res_time_wait=0;
		while (!flag)
		{
			if (pthread_mutex_trylock(&philosother_->right_fork->mutex))
			{							
				int time_wait=random()%10+1;
				sleep(time_wait);
				res_time_wait+=time_wait;
			}
			else if (pthread_mutex_trylock(&philosother_->left_fork->mutex))
			{
				pthread_mutex_unlock(&philosother_->right_fork->mutex);
				int time_wait=random()%10+1;
				sleep(time_wait);
				res_time_wait+=time_wait;
			}
			else
				flag=1;
		}
		global_wait_time+=res_time_wait;
		write (1, buff, sprintf(buff, "Philosopher %d wait fork at %d second\n", philosother_->id, res_time_wait));
		//Едим
		int time_eating = random()%10+1;
		global_eat_time+=time_eating;
		write (1, buff, sprintf(buff, "Philosopher %d eat at %d second\n", philosother_->id, time_eating));
		sleep(time_eating);
		//Кладем вилки
		pthread_mutex_unlock(&philosother_->right_fork->mutex);
		pthread_mutex_unlock(&philosother_->left_fork->mutex);
	}	
	write (1, buff, sprintf(buff, "Philosopher %d wait at %d second, eat at %d second, think at %d second and died\n", philosother_->id, global_wait_time, global_eat_time, global_think_time));
	pthread_exit(0);
}

int main(int argc, char **argv)
{
	//Разбор аргументов
	if (argc >3)
	{
		write(2, "Error with arguments", strlen("Error with arguments"));
		return ERARGUMENTS;
	}
	int count_philosopher=10, time_of_live_philosopher=100;
	if (argc>1)
		count_philosopher=atoi(argv[1]);
	if (argc>2)
		time_of_live_philosopher=atoi(argv[2]);	
	//Создание вилок
	struct fork forks[count_philosopher];
	int i=0;
	for (; i<count_philosopher; ++i)
	{
		forks[i].id=i;
		if (pthread_mutex_init(&forks[i].mutex, NULL))
			{
			int j=0;
			for (; j<i; ++j)
				pthread_mutex_destroy(&forks[i].mutex);
			write(2, "Error with mutex", strlen("Error with mutex"));
			return ERMUTEX;
			}
	}
	//Создание философов
	struct philosother philosothers[count_philosopher];
	i =0;
	for (;i<count_philosopher; ++i)
	{
		philosothers[i].id=i;
		philosothers[i].left_fork=forks+i;
		philosothers[i].right_fork=forks+i+1;			
	}
	philosothers[count_philosopher-1].right_fork=forks+0;
	//Инициализация жизни философов
	i=0;
	for (; i<count_philosopher; ++i)
	{
		if (pthread_create(&philosothers[i].thread, NULL, (void* (*)(void*))philosothers_live, philosothers+i)!=0)
		{
			int j=0; 
			for (; j<count_philosopher;++j)
			{
				pthread_kill(philosothers[i].thread, SIGKILL);
				pthread_mutex_destroy(&forks[j].mutex);
			}
			write(2, "Error with thread", strlen("Error with thread"));
			return ERTHREAD;
		}
	}
	//Начало жизни философов
	start=1;
	sleep(time_of_live_philosopher);
	//Конец жизни философов
	start=0;
	i=0;
	//Убийство философов
	for (; i<count_philosopher; ++i)
	{
		pthread_join(philosothers[i].thread, NULL);
	}
	for (; i<count_philosopher; ++i)
	{
		pthread_mutex_destroy(&forks[i].mutex);	
	}
	return 0;
}
