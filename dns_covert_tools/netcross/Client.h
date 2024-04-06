#include "globals.h"
#ifndef CLIENT_H_
#define CLIENT_H_
extern ConfParser cp;
class Client
{
private:
	unsigned short LinkID;
	Link* ReferringLink;
	Endpoint* ReferringEndpoint;
	bool authenticated;
	Endpoint * CreateEndpoint();
	class CLinkReceiver : public RecvNotifier {
		public:
		 Client *clientObj;
		private:
		 void NotifyRecv(Buffer&);	
	}LinkReceiver;
	class CEndpointReceiver : public RecvNotifier {
		public:
		 Client *clientObj;
		private: 
		 void NotifyRecv(Buffer&);	
	}EndpointReceiver;

public:
	Client(unsigned short,Link&);
	virtual ~Client();
	unsigned short GetLinkID();
	Link& GetLinkObj();
	Endpoint& GetEndpointObj();
	bool GetAuthStatus();
	void SetAuthStatus(bool);
	
};

#endif /*CLIENT_H_*/
