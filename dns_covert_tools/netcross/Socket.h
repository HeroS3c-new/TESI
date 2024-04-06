#include "globals.h"
#ifndef SOCKET_H_
#define SOCKET_H_

#define TCP SOCK_STREAM;
#define UDP SOCK_DGRAM;
#define RAW SOCK_RAW;
#define SOCK_BUF_SIZE 65535

typedef enum {SOCKRET_OK,SOCKRET_CLOSINGSOCK,SOCKRET_TIMEOUT,SOCKRET_ERROR} SockRetVal;

class Socket
{
	private:
	void constructor(int,int);
	int sock,type;
	struct sockaddr_in remoteEP;
	bool opened;
	unsigned short OperTimeout;
public:
	Socket(int,int);

	bool IsConnected();
	void BindToIP(int,string);
	void Connect(string,int);
	unsigned long SendData(Buffer &);
	unsigned long SendData(Buffer &,struct sockaddr_in*);
	SockRetVal RecvData(Buffer&,struct sockaddr_in *);
	SockRetVal RecvData(Buffer&);
	void SetOperationTimeout(unsigned short);
	void Listen(int);
	Socket * Accept();	
	bool isOpened();
	void close();
	void setTimeout(unsigned short);
	int getSockFD();
	void WaitForConnection();
	virtual ~Socket();
};

#endif /*SOCKET_H_*/
