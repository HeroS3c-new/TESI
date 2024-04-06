#ifndef RECVNOTIFIER_H_
#define RECVNOTIFIER_H_

class RecvNotifier {
public:
	RecvNotifier() {};
	virtual ~RecvNotifier() {};
	virtual void NotifyRecv(Buffer&)=0;
	
};
#endif /*RECVNOTIFIER_H_*/
