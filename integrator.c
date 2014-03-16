#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define ERPIPE -1
#define SUCSESS 0

long double math_function (long double x)
{
	return x*x*x;
}

long double integrator (long double a, long double b, long double dx, long double(*f)(long double))
{
	long double t=a, res=0;
	while (t<b)
	{
		res+=dx*f(t);
		t+=dx;
	}
	return res;
}

long double create_integrator (int count_process, long double a, long double b, long double dx, long double (*f)(long double))
{
//создание Pipe
	int descriptors[2];
	if (pipe(descriptors))
	{
		perror("Pipe_error: ");
		exit (ERPIPE);
	} 	
//Основной цикл
	int i=0;
	for (; i<count_process; ++i)
	{
			int processid = fork();
			if (processid==0)
			{
				close(descriptors[0]);
				//Основная функция
				long double res_integral=integrator(a+(b-a)/count_process*i, a+(b-a)/count_process*(i+1), dx, math_function);
				write(descriptors[1], &res_integral, sizeof(long double));
				close(descriptors[1]);
				//Умираем, чтобы не мешать основной программе
				exit(SUCSESS);
			}
			else if (processid<0) 
				perror("Fork_error: ");
	}
	close(descriptors[1]);
	long double result[count_process];
	i=0;
	for (;i<count_process; ++i)
		result[i]=0;
//Читаем результат из труб
	i=0;
	for (;i<count_process; ++i)
		if (read(descriptors[0], &result[i], sizeof(long double))!=sizeof(long double))
			write(2, "problem read from pipe\n", sizeof("probleam read from pipe\n"));
	close(descriptors[0]);
	
//Собираем воедино
	long double res=0;
	i=0;
	for (; i<count_process; ++i)
		res+=result[i];
	return res;
}


int main()
{
	double a=0, b=10, dx=0.000001;
	int count_integrator=10;
	char buff[sizeof(long double)];
	write(1, buff, sprintf(buff, "%Lf\n", create_integrator(count_integrator, a, b, dx, math_function)));
	return 0;
}
