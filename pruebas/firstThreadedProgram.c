#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void* acum(void* args);

struct acum_data
{
	int start;
	int end;
	int *dest;
};

int main()
{
	pthread_t thread1;
	pthread_t thread2;
	
	int sum = 0;
	
	struct acum_data thread_args = {0, 50, &sum};
	
	pthread_create(&thread1, NULL, acum, &thread_args);
	pthread_create(&thread2, NULL, acum, &thread_args);
	
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	
	printf("%d\n", sum);
}

void* acum(void* args)
{
	struct acum_data *data = (struct acum_data*) args;
	
	for (int i = data->start; i <= data->end; ++i)
	{
		pthread_mutex_lock(&lock);
		*(data->dest) += i;
		pthread_mutex_unlock(&lock);
	}
}
