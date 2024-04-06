#include "globals.h"
#ifndef ENDPOINT_H_
#define ENDPOINT_H_

extern char AppMode;
typedef enum {EPRET_OK, EPRET_NODATA, EPRET_CLOSED,EPRET_ERR} EP_Retval;
class Endpoint
{
protected:

	Endpoint(Client&);
	RecvNotifier *RNotif;
	Client *refClient;
public:
	virtual bool BroadcastClients() { return false;};
	map<string,string> Parameters;
	void RegisterRecvNotifier(RecvNotifier*);
	virtual void Init();
	virtual void Send(Buffer&)=0;
	virtual ~Endpoint();
	virtual vector<string> GetNeededParams()=0;
};

#endif /*ENDPOINT_H_*/
