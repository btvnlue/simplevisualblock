#include "CThreadPool.h"

int Log(const wchar_t* msg);

int CThTask::Process()
{
	return 0;
}

DWORD WINAPI ThProcPoolTask(_In_ LPVOID lpParameter)
{
	DWORD dwv;
	CThreadPool* self = reinterpret_cast<CThreadPool*>(lpParameter);
	CThTask* task;
	TaskBox* box;
	DWORD stk, etk;
	WCHAR cbuf[1024];

	while (self->keeprun)
	{
		dwv = ::WaitForSingleObject(self->hQueueFlag, 100);
		if (dwv == WAIT_OBJECT_0)
		{
			task = NULL;
			if (self->firsttask)
			{
				box = self->firsttask;
				self->firsttask = box->next;
				if (self->firsttask == NULL)
				{
					self->lasttask = NULL;
				}
				task = box->task;
				delete box;
				self->taskcount--;
			}
			::SetEvent(self->hQueueFlag);
			if (task)
			{
				stk = ::GetTickCount();
				InterlockedIncrement(&self->running);
				task->Process();
				InterlockedDecrement(&self->running);
				etk = ::GetTickCount();
				wsprintf(cbuf, L"Task proc tick: [%d]", etk - stk);
				Log(cbuf);
				delete task;
			}
			else {
				Sleep(100);
			}
		}
	}
	return 0x1234;
}

CThreadPool::CThreadPool()
	: keeprun(FALSE)
	, putcount(0)
	, firsttask(NULL)
	, lasttask(NULL)
	, taskcount(0)
	, running(0)
	, pause(FALSE)
{
	hQueueFlag = ::CreateEvent(NULL, FALSE, TRUE, NULL);
}

int CThreadPool::CreatePool(int poolsize)
{
	DWORD dwid;
	HANDLE hth;

	keeprun = TRUE;
	for (int ii = 0; ii < poolsize; ii++)
	{
		hth = ::CreateThread(NULL, 0, ThProcPoolTask, (LPVOID)this, 0, &dwid);
		threads.push_back(hth);
	}
	return 0;
}

int CThreadPool::PutTask(CThTask* task)
{
	DWORD dwv;
	TaskBox* box;
	if (keeprun)
	{
		InterlockedIncrement(&putcount);
		dwv = ::WaitForSingleObject(hQueueFlag, INFINITE);
		if (dwv == WAIT_OBJECT_0)
		{
			box = new TaskBox();
			box->task = task;
			box->next = NULL;
			if (lasttask)
			{
				lasttask->next = box;
			}
			lasttask = box;
			if (firsttask == NULL)
			{
				firsttask = box;
			}
			taskcount++;
			::SetEvent(hQueueFlag);
		}
		InterlockedDecrement(&putcount);
	}
	else
	{
		delete task;
	}
	
	return 0;
}

int CThreadPool::GetTasksCount()
{
	return taskcount + running;
}

int CThreadPool::Stop()
{
	keeprun = FALSE;
	DWORD dwti;

	if (pause) {
		Resume();
	}
	for (std::list<HANDLE>::iterator itth = threads.begin()
		; itth != threads.end()
		; itth++)
	{
		dwti = STILL_ACTIVE;
		while (dwti == STILL_ACTIVE)
		{
			::Sleep(100);
			::GetExitCodeThread(*itth, &dwti);
		}
		::CloseHandle(*itth);
	}
	threads.clear();
	while (putcount > 0)
	{
		::Sleep(100);
	}
	TaskBox* box;
	DWORD dwv = ::WaitForSingleObject(hQueueFlag, INFINITE);
	if (dwv == WAIT_OBJECT_0)
	{
		while (firsttask)
		{
			box = firsttask;
			firsttask = box->next;
			delete box->task;
			delete box;
		}
		::SetEvent(hQueueFlag);
	}
	return 0;
}

int CThreadPool::Pause()
{
	if (pause == FALSE) {
		if (::WaitForSingleObject(hQueueFlag, INFINITE) == WAIT_OBJECT_0) {
			pause = TRUE;
		}
	}
	return 0;
}

int CThreadPool::Resume()
{
	if (pause) {
		SetEvent(hQueueFlag);
		pause = FALSE;
	}
	return 0;
}
