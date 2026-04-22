#pragma once
#include "ESocket.h"
#include "EdoyunThread.h"

//Confirm the core functions
//Confirm the business logic

class ENetWork
{

};

typedef int (*AcceptFunc)(void* arg, ESOCKET client);
typedef int (*RecvFunc)(void* arg, const EBuffer& buffer);
typedef int (*SendFunc)(void* arg,ESOCKET& client,int ret);
typedef int (*RecvFromFunc)(void* arg, const EBuffer& buffer, ESockaddrIn& addr);
typedef int (*SendToFunc)(void* arg,const ESockaddrIn& addr,int ret);

class EServerParameter
{
public:
	EServerParameter(const std::string& ip, short port, ETYPE type);
	EServerParameter(
		const std::string& ip = "0.0.0.0",
		short port = 9527, ETYPE type = ETYPE::ETypeTCP,
		AcceptFunc acceptf = NULL,
		RecvFunc recvf = NULL,
		SendFunc sendf = NULL,
		RecvFromFunc recvfromf = NULL,
		SendToFunc sendtof = NULL
		
	);
	// input
	EServerParameter& operator<<(AcceptFunc func);
	EServerParameter& operator<<(RecvFunc func);
	EServerParameter& operator<<(SendFunc func);
	EServerParameter& operator<<(RecvFromFunc func);
	EServerParameter& operator<<(SendToFunc func);
	EServerParameter& operator<<(const std::string& ip);
	EServerParameter& operator<<(short port);
	EServerParameter& operator<<(ETYPE type);
	// output
	EServerParameter& operator>>(AcceptFunc& func);
	EServerParameter& operator>>(RecvFunc& func);
	EServerParameter& operator>>(SendFunc& func);
	EServerParameter& operator>>(RecvFromFunc& func);
	EServerParameter& operator>>(SendToFunc& func);
	EServerParameter& operator>>(std::string& ip);
	EServerParameter& operator>>(short& port);
	EServerParameter& operator>>(ETYPE& type);
	// Copy constructor, assignment operator overloading, 
	// used for value assignment of the same type
	EServerParameter(const EServerParameter& param);
	EServerParameter& operator=(const EServerParameter& param);

	std::string m_ip;
	short m_port;
	ETYPE m_type;
	AcceptFunc m_accept;
	RecvFunc m_recv;
	SendFunc m_send;
	RecvFromFunc m_recvfrom;
	SendToFunc m_sendto;
};

class EServer:public ThreadFuncBase
{
public:
	// When to set the key parameters depends on one's own development experience and actual needs, and should be adjusted accordingly.
	EServer(const EServerParameter& param);
	~EServer();
	int Invoke(void* arg);
	int Send(ESOCKET& client, const EBuffer& buffer);
	int Sendto(ESockaddrIn& addr,const EBuffer& buffer);
	int Stop();
private:
	int threadFunc();
	int theadUDPFunc();
	int threadTCPFunc();
private:
	EServerParameter m_params;
	void* m_args;
	EdoyunThread m_thread;
	ESOCKET m_sock;
	std::atomic<bool> m_stop;
};
