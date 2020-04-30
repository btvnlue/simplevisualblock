#pragma once
#include <Windows.h>
#include <list>

class CThTask
{
public:
	virtual int Process();
};

class TaskBox
{
public:
	CThTask* task;
	TaskBox* next;
};
class CThreadPool
{
public:
	CThreadPool();

	BOOL keeprun;
	long putcount;
	long taskcount;
	unsigned long running;
	HANDLE hQueueFlag;
	//std::list<CThTask*> tasks;
	TaskBox* firsttask;
	TaskBox* lasttask;
	std::list<HANDLE> threads;

	int CreatePool(int poolsize);
	int PutTask(CThTask* task);
	int GetTasksCount();
	int Stop();
};

