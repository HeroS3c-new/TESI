#include "globals.h"
#ifndef LINK_H_
#define LINK_H_

extern char AppMode;

class Link
{
protected:
	map<unsigned short, RecvNotifier* > NotifyMap;
	Link();
	bool estabilished;
	unsigned short LinkID;
public:
	map<string,string> Parameters;
	Link *me;
	virtual void Init()=0;
	virtual void Terminate()=0;
	virtual void Send(Buffer&,unsigned short)=0;
	virtual void ThrottleFeedback(bool){};
	virtual ~Link();
	virtual vector<string> GetNeededParams()=0;
	virtual void RegisterRecvNotifier(Client&,RecvNotifier*); 
	virtual void UnRegisterRecvNotifier(Client&); 
	virtual void UnRegisterRecvNotifier(unsigned short); 
};


#endif /*LINK_H_*/
