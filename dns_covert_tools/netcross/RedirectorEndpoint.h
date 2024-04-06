#include "globals.h"
#ifndef REDIRECTORENDPOINT_H_
#define REDIRECTORENDPOINT_H_

class RedirectorEndpoint : public Endpoint
{
private:
	Socket *sock;
	Socket *clientSock;
	Thread<RedirectorEndpoint> THDRecv;
	Thread<RedirectorEndpoint> THDAcc;
	/* These parameters will be send by the client upon his Init() method */
	string remoteHost;
	int remotePort;
	void SendCliParamsToServer();
	void SendState(unsigned char);
	void SendError(string);
	bool connected;
	void RecvThread();
	void WaitIncomingConnection();

public:
	RedirectorEndpoint(Client& iClient) :Endpoint(iClient) {};;
	virtual ~RedirectorEndpoint();
	/* Fyrst Byte indicator when exchanging data*/
	static const char CNT_Params='p';
	static const char CNT_Data='d';
	static const char CNT_Ready='r';
	static const char CNT_Error='e';
	static const char CNT_Close='c';
	static const char CNT_Newconn='n';
	static const char CNT_Connready='x';	
	void Init();
	void Send(Buffer&);
	bool IsConnected();
	
	vector<string> GetNeededParams();
};

#endif /*REDIRECTORENDPOINT_H_*/
