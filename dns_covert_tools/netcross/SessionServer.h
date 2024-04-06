#include "globals.h"
#ifndef SESSIONSERVER_H_
#define SESSIONSERVER_H_

#define SESSION_TIMEOUT 120
typedef map<unsigned short,Client *> TClientMap;
typedef map<unsigned short,unsigned long> TTimeoutMap;

class SessionServer
{
private:
	TClientMap clients; //MAP: linkID->client
	TTimeoutMap TimeoutMap;
	Thread<SessionServer> THDTimeout;
	SessionServer();
	virtual ~SessionServer();
	static SessionServer *singletonObj;
	void TimeoutScanThread();
	unsigned short FindFreeID();
public:
	static SessionServer& GetInstance();
	TClientMap GetClientMap();

	unsigned short NewClient(Link&);
	void RemoveClient(unsigned short);
	void ClientAlive(unsigned short);
	bool ClientExists(unsigned short);
	Client& GetClient(unsigned short);
};

#endif /*SESSIONSERVER_H_*/
