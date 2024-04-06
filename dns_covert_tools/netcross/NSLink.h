#include "globals.h"
#ifndef NSLINK_H_
#define NSLINK_H_

#define POLL_PREFIX "poll"
#define NEWLINK_PREFIX "newlink"
#define LINKID_PREFIX "LNKID"
#define ENDLINK_PREFIX "ENDOL"
#define SOCK_TIMEOUT 5
#define LINK_TIMEOUT 2
#define LINKID_LENGTH 3
#define SEQID_LENGTH 5
#define POLL_USEC 250000
#define INIT_POLL_RATIO 2
#define POLL_RATIO_MAX 5
#define THROTTLE_DECREASE_FEEDBACK -3
#define THROTTLE_INCREASE_LIMIT 4
#define FEEDBACK_POSITIVE_RATIO 30



struct DNSReplyItem{
	DNSPacket QueryPack;
	unsigned short LinkID;
	struct sockaddr_in endpoint;
};

class NSLink: public Link
{
private:
	Socket *sock;
	Thread<NSLink> pollingThread;
	Thread<NSLink> replyingThread;
	Thread<NSLink> recThread;
	pthread_mutex_t DNSReplyMutex;
	map<unsigned short, list<DNSPacket> > txQueue;
	map<unsigned short, map <unsigned short, string  > > ServerRxQueue; //ServerRxQueue[LinkID][TransactionID]
	map<unsigned short, Buffer> ClientRxQueue; //ClientRxQueue[TransactionID]
	unsigned short SendingSeq; //Used to cycle the first 4 bit of the Sending Sequence ID (or transaction id in case of client)
	static unsigned short TIDTracker;
	unsigned char PollRatio;
	unsigned short LastSeqID;
	static const int MAX_QUERY_TOTSIZE=240;
	static const int MAX_ZONE_LENGTH=32;
	string BaseHost;
	unsigned short MaxPacketSize;
	string LinkIDToAscii();
	string SeqIDToAscii(unsigned short);
	bool isSending;
	unsigned short AsciiToLinkID(string&);
	void ReplyToClient(unsigned short,DNSPacket&,struct sockaddr_in *);	
	void LinkToServer();
	void PollForData();
	list<DNSReplyItem> DNSReplyQueue;
	void DNSReplyingThread();
public:
	NSLink();
	virtual ~NSLink();
	void Init();	
	void Terminate();
	void Send(Buffer&,unsigned short);
	void PurgeDots(string&);
	void PollingThread();
	void RecvThread();
	void RegisterRecvCallback(Client&,void (*)(Buffer&));
	void ThrottleFeedback(bool);
	vector<string> NSLink::GetNeededParams();
};

#endif /*NSLINK_H_*/
