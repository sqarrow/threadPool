#include"ThreadPool.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>

const int NUMBER = 2;

// 1.创建线程池并初始化
ThreadPool* threadPoolCreate(int minNum, int maxNum, int queueCapacity)
{
	/*ThreadPool* pool = (ThreadPool*)malloc(sizeof(ThreadPool));
	if (pool == NULL)
	{
		printf("pool malloc() is fail\n");
		return NULL;
	}
	memset(pool, 0, sizeof(ThreadPool));
	
	do
	{
		pool->maxNum = maxNum;
		pool->queueCapacity = queueCapacity;
		pool->taskQ = (Task*)malloc(sizeof(Task) * queueCapacity);
		if (pool->taskQ == NULL)
		{
			printf("Task malloc() is fail\n");
			break;
		}
		memset(pool->taskQ, 0, sizeof(Task) * queueCapacity);
		pool->queueSize = 0;
		pool->queueFront = 0;
		pool->queueRear = 0;
		pool->shutdown = 0;

		pthread_create(&pool->managerID, NULL, manager, NULL);
		pool->threadIDs = (pthread_t*)malloc(sizeof(pthread_t) * maxNum);
		if (pool->threadIDs == NULL)
		{
			printf("threadIDs malloc() is fail\n");
			break;
		}
		memset(pool->threadIDs, 0, sizeof(pthread_t) * maxNum);
		for (int i = 0; i < minNum; ++i)
		{
			pthread_create(&pool->threadIDs[i], NULL, worker, pool);
			pool->aliveNum++;
		}
		pool->minNum = minNum;
		pool->exitNum = 0;

		if (pthread_mutex_init(&pool->mutexBusy, NULL) != 0 || pthread_mutex_init(&pool->mutexPool, NULL) != 0 ||
			pthread_cond_init(&pool->notEmpty, NULL) != 0 || pthread_cond_init(&pool->notFull, NULL) != 0)
		{
			printf("mutex or condition inital failed\n");
			break;
		}

		return pool;
	} while (0);

	if (pool->taskQ) free(pool->taskQ);
	if (pool->threadIDs) free(pool->threadIDs);
	free(pool);

	return NULL;*/
	ThreadPool* pool = (ThreadPool*)malloc(sizeof(ThreadPool));
	do
	{
		if (pool == NULL)
		{
			printf("malloc threadpool fail...\n");
			break;
		}

		pool->threadIDs = (pthread_t*)malloc(sizeof(pthread_t) * maxNum);
		if (pool->threadIDs == NULL)
		{
			printf("malloc threadIDs fail...\n");
			break;
		}
		memset(pool->threadIDs, 0, sizeof(pthread_t) * maxNum);
		pool->minNum = minNum;
		pool->maxNum = maxNum;
		pool->busyNum = 0;
		pool->aliveNum = minNum;    // 和最小个数相等
		pool->exitNum = 0;

		if (pthread_mutex_init(&pool->mutexPool, NULL) != 0 ||
			pthread_mutex_init(&pool->mutexBusy, NULL) != 0 ||
			pthread_cond_init(&pool->notEmpty, NULL) != 0 ||
			pthread_cond_init(&pool->notFull, NULL) != 0)
		{
			printf("mutex or condition init fail...\n");
			break;
		}

		// 任务队列
		pool->taskQ = (Task*)malloc(sizeof(Task) * queueCapacity);
		pool->queueCapacity = queueCapacity;
		pool->queueSize = 0;
		pool->queueFront = 0;
		pool->queueRear = 0;

		pool->shutdown = 0;

		// 创建线程
		pthread_create(&pool->managerID, NULL, manager, pool);
		for (int i = 0; i < minNum; ++i)
		{
			pthread_create(&pool->threadIDs[i], NULL, worker, pool);
		}
		return pool;
	} while (0);

	// 释放资源
	if (pool && pool->threadIDs) free(pool->threadIDs);
	if (pool && pool->taskQ) free(pool->taskQ);
	if (pool) free(pool);

	return NULL;
}
// 2.销毁线程池
int threadPoolDestory(ThreadPool* pool)
{
	if (pool == NULL) return -1;
	
	// 关闭线程池 shutdown = 1
	//pthread_mutex_lock(&pool->mutexPool);
	pool->shutdown = 1;
	//pthread_mutex_unlock(&pool->mutexPool);

	// 等待管理者线程
	pthread_join(pool->managerID, NULL);

	// 唤醒消费者线程(工作线程) 将其退出
	for (int i = 0; i < pool->aliveNum; ++i)
	{
		pthread_cond_signal(&pool->notEmpty);
	}

	// 释放内存
	if (pool->taskQ != NULL)
	{
		free(pool->taskQ);
		pool->taskQ = NULL;
	}
	if (pool->threadIDs != NULL)
	{
		free(pool->threadIDs);
		pool->threadIDs = NULL;
	}

	pthread_mutex_destroy(&pool->mutexBusy);
	pthread_mutex_destroy(&pool->mutexPool);

	pthread_cond_destroy(&pool->notEmpty);
	pthread_cond_destroy(&pool->notFull);

	free(pool);
	pool = NULL;

	return 0;
	//if (pool == NULL)
	//{
	//	return -1;
	//}

	//// 关闭线程池
	//pool->shutdown = 1;
	//// 阻塞回收管理者线程
	//pthread_join(pool->managerID, NULL);
	//// 唤醒阻塞的消费者线程
	//for (int i = 0; i < pool->aliveNum; ++i)
	//{
	//	pthread_cond_signal(&pool->notEmpty);
	//}
	//// 释放堆内存
	//if (pool->taskQ)
	//{
	//	free(pool->taskQ);
	//}
	//if (pool->threadIDs)
	//{
	//	free(pool->threadIDs);
	//}

	//pthread_mutex_destroy(&pool->mutexPool);
	//pthread_mutex_destroy(&pool->mutexBusy);
	//pthread_cond_destroy(&pool->notEmpty);
	//pthread_cond_destroy(&pool->notFull);

	//free(pool);
	//pool = NULL;

	//return 0;
}
// 3.给线程池增加任务
void threadPoolAdd(ThreadPool* pool, void (*func)(void*), void* arg)
{
	if (pool == NULL) return;
	pthread_mutex_lock(&pool->mutexPool);
	while (pool->queueSize == pool->queueCapacity && pool->shutdown == 0)
	{
		// 阻塞生产者
		pthread_cond_wait(&pool->notFull, &pool->mutexPool);
		
	}
	if (pool->shutdown == 1)
	{
		pthread_mutex_unlock(&pool->mutexPool);
		return ;
	}

	// 添加任务
	pool->taskQ[pool->queueRear].arg = arg;
	pool->taskQ[pool->queueRear].func = func;
	pool->queueRear = (pool->queueRear + 1) % pool->queueCapacity;
	pool->queueSize++;
	
	pthread_cond_signal(&pool->notEmpty);

	pthread_mutex_unlock(&pool->mutexPool);
	//pthread_mutex_lock(&pool->mutexPool);
	//while (pool->queueSize == pool->queueCapacity && !pool->shutdown)
	//{
	//	// 阻塞生产者线程
	//	pthread_cond_wait(&pool->notFull, &pool->mutexPool);
	//}
	//if (pool->shutdown)
	//{
	//	pthread_mutex_unlock(&pool->mutexPool);
	//	return;
	//}
	//// 添加任务
	//pool->taskQ[pool->queueRear].func = func;
	//pool->taskQ[pool->queueRear].arg = arg;
	//pool->queueRear = (pool->queueRear + 1) % pool->queueCapacity;
	//pool->queueSize++;

	//pthread_cond_signal(&pool->notEmpty);
	//pthread_mutex_unlock(&pool->mutexPool);
}
// 4.获取线程池中工作的线程的个数
int threadPoolBusyNum(ThreadPool* pool)
{
	if (pool == NULL) return -1;
	int sum = 0;
	pthread_mutex_lock(&pool->mutexPool);
	sum = pool->busyNum;
	pthread_mutex_unlock(&pool->mutexPool);

	return sum;
}
// 5.获取线程池中活着的线程的个数
int threadPoolAliveNum(ThreadPool* pool)
{
	if (pool == NULL) return -1;
	int sum = 0;
	pthread_mutex_lock(&pool->mutexPool);
	sum = pool->aliveNum;
	pthread_mutex_unlock(&pool->mutexPool);
	return sum;
}

void* worker(void* arg)
{
	ThreadPool* pool = (ThreadPool*)arg;
	while (1)
	{
		pthread_mutex_lock(&pool->mutexPool);
		// 判断队列是否为空
		while (pool->queueSize == 0 && !pool->shutdown)
		{
			// 阻塞工作线程
			pthread_cond_wait(&pool->notEmpty, &pool->mutexPool);
			// 判断是不是要销毁线程
			if (pool->exitNum > 0)
			{
				pool->exitNum--;
				if (pool->aliveNum > pool->minNum)
				{
					pool->aliveNum--;
					pthread_mutex_unlock(&pool->mutexPool);
					threadExit(pool);
				}
			}
		}
		// 判断线程池是否被关闭了
		if (pool->shutdown)
		{
			pthread_mutex_unlock(&pool->mutexPool);
			threadExit(pool);
		}

		// 从任务队列中取出一个任务
		Task task;
		task.arg = pool->taskQ[pool->queueFront].arg;
		task.func = pool->taskQ[pool->queueFront].func;
		// 移动头结点
		pool->queueFront = (pool->queueFront + 1) % pool->queueCapacity;
		pool->queueSize--;
		// 解锁
		pthread_cond_signal(&pool->notFull);
		pthread_mutex_unlock(&pool->mutexPool);
		printf("thread %ld start working\n", pthread_self());
		pthread_mutex_lock(&pool->mutexBusy);
		pool->busyNum++;
		pthread_mutex_unlock(&pool->mutexBusy);
		task.func(task.arg);
		free(task.arg);
		task.arg = NULL;
		printf("thread %ld end working...\n", pthread_self());
		pthread_mutex_lock(&pool->mutexBusy);
		pool->busyNum--;
		pthread_mutex_unlock(&pool->mutexBusy);
	}
	return NULL;
}
void* manager(void* arg)
{
	if (arg == NULL) return NULL;
	ThreadPool* pool = (ThreadPool*)arg;
	
	int queueSize, liveNum, busyNum, counter;

	while (pool->shutdown == 0)
	{
		sleep(3);

		// 区
		pthread_mutex_lock(&pool->mutexPool);
		queueSize = pool->queueSize;
		liveNum = pool->aliveNum;
		pthread_mutex_unlock(&pool->mutexPool);

		// 工作线程数
		pthread_mutex_lock(&pool->mutexBusy);
		busyNum = pool->busyNum;
		pthread_mutex_unlock(&pool->mutexBusy);

		// 添加线程 每次添加 2 个
		// 任务个数 > 存活的线程数  &&  存活的线程数 < 最小的线程数
		if (queueSize > liveNum && liveNum < pool->minNum)
		{
			pthread_mutex_lock(&pool->mutexPool);
			counter = 0;
			for (int i = 0; i < pool->maxNum && counter < NUMBER && pool->aliveNum < pool->maxNum; ++i)
			{
				if (pool->threadIDs[i] == 0)
				{
					pthread_create(&pool->threadIDs[i], NULL, worker, pool);
					counter++;
					pool->aliveNum++;
				}
			}
			pthread_mutex_unlock(&pool->mutexPool);
		}

		// 销毁线程
		// 忙的线程 * 2 < 存活的线程数  && 存活的线程数 > 最小线程数
		if (busyNum * 2 < liveNum && liveNum > pool->minNum)
		{
			pthread_mutex_lock(&pool->mutexPool);
			pool->exitNum = NUMBER;
			pthread_mutex_unlock(&pool->mutexPool);
			
			// 让工作的线程自杀
			for (int i = 0; i < NUMBER; ++i)
			{
				pthread_cond_signal(&pool->notEmpty);
			}
		}
	}
	return NULL;
}
void threadExit(ThreadPool* pool)
{
	if (pool == NULL) return;
	pthread_t tid = pthread_self();
	for (int i = 0; i < pool->maxNum; ++i)
	{
		if (tid == pool->threadIDs[i])
		{
			pool->threadIDs[i] = 0;
			printf("thread %ld is exiting...\n", tid);
			break;
		}
	}
	pthread_exit(NULL);
}



