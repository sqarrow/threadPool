#ifndef __THREADPOOL__
#define __THREADPOOL__

#include<pthread.h>

typedef void (*ThreadFun)(void*);
typedef struct task
{
	ThreadFun func;
	void* arg;
}Task;

// typedef struct threadPoll
// {
//     Task* taskQ;        // 任务队列
//     int queueCapacity;      // 任务队列的容量
//     int queueSize;      // 当前任务个数
//     int queueFront;     // 队头  --  取数据
//     int queueRear;      // 队尾  --  放数据
//     pthread_t managerID;        // 管理者线程
//     pthread_t* threadIDS;       // 工作线程
//     int minNum;         // 最小线程数
//     int maxNum;         // 最大线程数
//     int busyNum;        // 工作线程数
//     int liveNum;        // 存活的总线程数
//     int exitNum;        // 要销毁的线程数
//     pthread_mutex_t mutexPoll;      // 锁整个线程池
//     pthread_mutex_t mutexBusy;      // 锁工作线程数
//     pthread_cond_t notFull;         // 生产者 - 消费者模型 实现队列任务的 生产和消耗
//     pthread_cond_t notEmpty;        //
//     int shutdown;       // 是否要销毁线程池 销毁为 1, 不消毁为 0
// }ThreadPoll;

typedef struct threadPool
{
	// 任务队列
	Task* taskQ;
	int queueCapacity;		// 任务队列的最大容量
	int queueSize;			// 任务队列当前的任务数
	int queueFront;			// 队列头
	int queueRear;			// 队列尾

	// 线程
	pthread_t managerID;		// 管理者线程
	pthread_t* threadIDs;		// 工作线程
	int minNum;				// 最小线程数 
	int maxNum;				// 最大线程数
	int busyNum;			// 工作线程数
	int aliveNum;			// 存活线程数
	int exitNum;			// 销毁线程数

	// 同步
	pthread_mutex_t mutexPool;
	pthread_mutex_t mutexBusy;
	pthread_cond_t notEmpty;
	pthread_cond_t notFull;
	
	// 退出
	int shutdown;
}ThreadPool;


// 1.创建线程池并初始化
ThreadPool* threadPoolCreate(int minNum, int maxNum, int queueCapacity);
// 2.销毁线程池
int threadPoolDestory(ThreadPool* pool);
// 3.给线程池增加任务
void threadPoolAdd(ThreadPool* pool, void (*func)(void*), void* arg);
// 4.获取线程池中工作的线程的个数
int threadPoolBusyNum(ThreadPool* pool);
// 5.获取线程池中活着的线程的个数
int threadPoolAliveNum(ThreadPool* pool);

void* worker(void* arg);
void* manager(void* arg);
void threadExit(ThreadPool* pool);



#endif

