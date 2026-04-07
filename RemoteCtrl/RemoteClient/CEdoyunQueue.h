#pragma once
template<class T>
class CEdoyunQueue
{
	//Thread-safe queue (implemented using IOCP)
public:
	CEdoyunQueue();
	~CEdoyunQueue();
	void PushBack();
	void PopFront(T& data);
	size_t Size();
	void Clear();
private:
	static void threadEntry(void* arg);
	void threadMain();
private:
	std::list<T> m_lstData;
	HANDLE m_hCompletionPort;
	HANDLE m_hThread;
public:
	typedef struct IocpParam
	{
		int nOperator;			//operator
		T strData;	//data
		_beginthread_proc_type cbFunc;	//callback
		HANDLE hEvent;	//The requirements for the pop operation
		IocpParam(int op, const char* sData, _beginthread_proc_type cb = NULL)
		{
			nOperator = op;
			strData = sData;
		}
		IocpParam()
		{
			nOperator = -1;
		}
	}PPARAM;	//Post Parameter: The structure used for transmitting information

	enum
	{
		EQPush,
		EQPop,
		EQSize,
		EQClear

	};
};



