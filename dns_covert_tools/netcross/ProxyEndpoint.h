#include "globals.h"
#ifndef PROXYENDPOINT_H_
#define PROXYENDPOINT_H_

#define PROXY_DEFAULT_PORT 8080
#define ERR_500_STR "HTTP/1.1 500 Server Error\nConnection: close\n\nServer Error"


class ProxyEndpoint : public Endpoint
{
private:
	Socket *sock;
	Socket *clientSock;
	
	Thread<ProxyEndpoint> THDRecv;
	
	void RecvThread();
	void ClientRecvThread();
	void SrvRecvThread();
	void PushBuffer(Buffer& iData);
	bool ParseHttpRequest(Buffer&,string &);
public:
	ProxyEndpoint(Client& iClient) :Endpoint(iClient) {};
	virtual ~ProxyEndpoint();
	void Init();
	void Send(Buffer&);
	
	vector<string> GetNeededParams();

	static const char CTL_INIT=0x10;
	static const char CTL_CLOSE=0x20;	
	static const char CTL_ERR=0x30;
	static const char CTL_REQ=0x40;
	static const char CTL_DATA=0x50;

};

#endif /*PROXYENDPOINT_H_*/
