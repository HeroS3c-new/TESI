#include "globals.h"
#ifndef DNSPACKET_H_
#define DNSPACKET_H_

#define DNSQUERY_SIZE 5
#define DNSANSWER_SIZE  11
class DNSPacket
{
public:
	/* Constants are reversed to avoid htons() BIG-ENDIAN conversion */
	static const unsigned short FLAGS_QUERY=0x0001;
	static const unsigned short FLAGS_RESPOK=0x8081;
	static const unsigned short FLAGS_NOSUCHNAME=0x8381;
	
	static const unsigned short TYPE_TXT=0x1000;
	static const unsigned short TYPE_A=0x0100;
	static const unsigned short CLASS_IN=0x0100;
	
	struct DNSHeader{
		unsigned short  TransID; 
		unsigned short  Flags;
		unsigned short  Queries;
		unsigned short  AnswerRR;
		unsigned short  AuthRR;
		unsigned short  AdditRR;
			
	} __attribute__((__packed__));
	
	struct DNSQuery {
		string Record;
		unsigned short Type;
		unsigned short Class;
	} __attribute__((__packed__));
	
	struct DNSAnswer {
		unsigned short ID;
		unsigned short Type;
		unsigned short Class;
		unsigned int TTL;
		unsigned short DataLen;
		Buffer data;
	} __attribute__((__packed__));

	struct DNSHeader header;
	struct DNSQuery query;
	struct DNSAnswer answer;
	
	DNSPacket();
	DNSPacket(Buffer&);
	Buffer Build();
	void Parse(Buffer&);
	virtual ~DNSPacket();

private:
	void GenRecord(string &,string &,int);
	void DecodeRecord(string &,string &,int);	
};

#endif /*DNSPACKET_H_*/
