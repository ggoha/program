#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <gmp.h>
#include <gmpxx.h>
#include <queue>

//Возвращаемые значения
#define ERTHREAD -1
#define ERARGUMENT -2
#define ERMUTEX -3


struct data
{
	mpz_t A, g, p;
	data(mpz_t g_, mpz_t p_, mpz_t A_)
	{
		mpz_init_set(g, g_); mpz_init_set(p, p_);	mpz_init_set(A, A_);
	}
};

struct thread_queue
{
	std::queue<data> q;
	pthread_mutex_t lock;
	pthread_cond_t cond;
	thread_queue()
	{
		if (pthread_mutex_init(&lock, NULL)<0)
		{
			printf("Eroror mutex initialization");
			exit(ERMUTEX);
		}
		if (pthread_cond_init(&cond, NULL)<0)
		{
			printf("Eroror cond initialization");
			exit(ERMUTEX);
		}
	}
	void push(struct data d)
	{
		pthread_mutex_lock(&lock);
		q.push(d);		
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&lock);
	} 
	struct data pop(void)
	{
		pthread_mutex_lock(&lock);
		if (q.empty())
			pthread_cond_wait(&cond, &lock);
		struct data d = q.front();
		q.pop();
		pthread_mutex_unlock(&lock);
		return d;
	}
};

void find_diskret_log(struct data d)
{
	mpz_t g, p, A;
	mpz_init_set(g, d.g); mpz_init_set(p, d.p); mpz_init_set(A, d.A);
	mpz_t A_test, power;
	mpz_init_set_ui(A_test, 1); mpz_init_set_ui(power, 0);
	while (mpz_cmp(A, A_test))
	{
//		mpz_out_str(stdout, 10, A_test);
//		printf("\n");
//		fflush(stdout);
		mpz_mul(A_test, A_test, g);
		mpz_mod(A_test, A_test, p);
		mpz_add_ui(power,power,1);
	}
	mpz_out_str(stdout, 10, g);
	printf("^");
	mpz_out_str(stdout, 10, power);
	printf("=");
	mpz_out_str(stdout, 10, A);
	printf("mod");
	mpz_out_str(stdout, 10, p);
	printf("\n");
	fflush(stdout);
}

void work(void* arg)
{
	struct thread_queue* q = (thread_queue*)arg;
	while (1)
	{
		struct data d = q->pop();
//		printf("Начал обрабатывать ");
//		mpz_out_str(stdout, 10, d.g);
//		printf(" ");
//		mpz_out_str(stdout, 10, d.p);
//		printf(" ");
//		mpz_out_str(stdout, 10, d.A);
//		printf("\n");
//		fflush(stdout);
		find_diskret_log(d);
	}
}

#define MAXSIZE_base 1024
#define MAXSIZE_mod 1024
#define MAXSIZE_answer 1024


int main (int argc, char** argv)
{
	//Разбор аргументов
	if (argc>2)
	{
		write(2, "Too many arguments\n", sizeof("Too many arguments\n"));
		return ERARGUMENT;
	}
	int count_worker =10;
	if (argc>1)
		count_worker=atoi(argv[1]);

	//Создаем очередь
	struct thread_queue q;
	//Создаем буферы под длинную арифметику
	char gs[MAXSIZE_base], ps[MAXSIZE_mod], As[MAXSIZE_answer]; 
	//Создаем длинную арифметику
	mpz_t g, p, A;
	//Флаги корректонсти преобразования строки в длинную арифметику
	bool flagg, flagp, flagA;
	//Инициализация длинной арифметики
	mpz_init(g); mpz_init(p); mpz_init(g);
	
	pthread_t workers_thread[count_worker];
	for (int i=0; i<count_worker; ++i)
		if (pthread_create(workers_thread+i ,NULL, (void* (*)(void*))work, (void*)&q) !=0 )
		{
			//Если не удалось создать нити убиваем уже созданные
			for (int j=0; j<i; ++j)
				pthread_kill(workers_thread[i], SIGKILL);
			return ERTHREAD;
		}
	while (1)
	{
		scanf("%1024s %1024s %1024s", &gs, &ps, &As);
		flagg = mpz_set_str(g, gs, 10); 		
		flagp = mpz_set_str(p, ps, 10); 		
		flagA = mpz_set_str(A, As, 10); 		
		if (!(flagg && flagp && flagA))
			q.push(data(g, p, A));
		else
			write(1, "Not correct numbers\n", sizeof("Not correct numbers\n"));
	}
}
