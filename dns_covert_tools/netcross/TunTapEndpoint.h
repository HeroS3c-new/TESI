#include "globals.h"
#ifndef TUNTAPENDPOINT_H_
#define TUNTAPENDPOINT_H_

#define TUNTAPDEV "/dev/net/tun"
#define TUNTAP_RSIZE 0xffff
#define TUNTAP_IFSCRIPT_UP 0
#define TUNTAP_IFSCRIPT_DOWN 1
class TunTapEndpoint : public Endpoint
{
private:
	static int tapType;
	static int SfdTap;
	static Thread<TunTapEndpoint> STHDRec;
	static string StapDev;
	int *fdTap;
    Thread<TunTapEndpoint> *THDRec;
    string *tapDev;
	void OpenInterface();
	void RecvThread();
	void IfScript(char);

public:
	TunTapEndpoint(Client& iClient) :Endpoint(iClient) {};
	virtual ~TunTapEndpoint();
	bool BroadcastClients();
	void Init();
	void Send(Buffer&);	
	vector<string> GetNeededParams();	
};


#endif /*TUNTAPENDPOINT_H_*/
