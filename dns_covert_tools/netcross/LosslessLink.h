#include "globals.h"

#ifndef LOSSLESSLINK_H_
#define LOSSLESSLINK_H_

#define MAX_LOSSLESS_QUEUE_LEN 24
#define MAX_LOSSLESS_RETRANS_TRIES 5
#define LOSSLESS_ACK_TIMEOUT 1
#define LOSSLESS_MAX_TIMEOUTS 3
#define LOSSLESS_INIT_WSIZE 10

typedef map<unsigned short,Buffer> TBufferMap;
typedef map<unsigned short,TBufferMap  > TQueueMap;

class LosslessReceiver : public RecvNotifier {
	friend class LosslessLink;
	private: 

	 RecvNotifier *baseNotifier;
	 Link  *baseLink;
	 LosslessLink *losLink;
	 Client *refClient;
	 void NotifyRecv(Buffer&);	
};


class LosslessLink : public Link , public TimerCallback
{
	friend class LosslessReceiver;
private:
	Link* baseLink;
	TQueueMap txBuf;
	TQueueMap rxBuf;
	unsigned short wSize;
	bool SedingLocked;
	
	Timer timerObj;

	map<unsigned short, unsigned short> lastSent;
	map<unsigned short, unsigned short> lastACK;	
	map<unsigned short, unsigned short> lastRecv;
	map<unsigned short,LosslessReceiver > MiddleNotifyMap;
	map<unsigned short,pthread_mutex_t > MutexMap;
	map< unsigned int, unsigned char > TimeoutsMap;
	void SendToBase(Buffer&,unsigned short,unsigned short,byte);
	void LockLinkMutex(unsigned short,bool );	
	void UnlockLinkMutex(unsigned short );	
	void TimerEvent(unsigned int);
	unsigned int GetTimerID(unsigned short, unsigned short);
	void GetParamsFromTimer(unsigned int, unsigned short &,unsigned short &);
public:
	LosslessLink(Link&);
	virtual ~LosslessLink();
	void Init();
	void Terminate();
	void Send(Buffer&,unsigned short);
	vector<string> GetNeededParams();	
	void RegisterRecvNotifier(Client&,RecvNotifier*);
	void UnRegisterRecvNotifier(Client&);
	void UnRegisterRecvNotifier(unsigned short);
	

	static const char CTL_Send=0x10;
	static const char CTL_RecvAndLost=0x20;
	static const char CTL_RecvAndACK=0x30;
	static const char CTL_Recvd=0x40;
	static const char CTL_ACK=0x50;
	static const char CTL_Lost=0x60;
	static const char CTL_Frameloss=0x70;
		
};

#endif /*LOSSLESSLINK_H_*/
