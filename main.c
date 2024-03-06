#include"ThreadPool.h"
#include<stdio.h>
#include<unistd.h>
#include<pthread.h>

void func(void* arg)
{
	int num = *((int*)arg);
	printf("thread %ld is working, number = %d\n", pthread_self(), num);
	sleep(1);
}

int main(int argc, char** argv)
{
	int kkk = 10;
	printf("%d\n", kkk);

	ThreadPool* pool = threadPoolCreate(3, 10, 100);
	for (int i = 0; i < 100; ++i)
	{
		int* num = (int*)malloc(sizeof(int));
		*num = i + 100;
		threadPoolAdd(pool, func, num);
	}

	sleep(10);
	threadPoolDestory(pool);
	printf("ending.......\n");
	return 0;
}

