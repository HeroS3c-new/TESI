#include "DNSPacket.h"

DNSPacket::DNSPacket()
{
	bzero(&header,sizeof(header));
}
DNSPacket::DNSPacket(Buffer &data){
	Parse(data);	
}

void DNSPacket::Parse(Buffer& data){
	byte *bBuf;
	byte *bZ;
	byte *bI;
	unsigned short int nQuery,nResp;

	string::size_type dataSize=data.size();
	if(dataSize<sizeof(DNSHeader))
		throw new Exception("Malformed DNS datagram");
	
	bBuf=data.toArray();

	memcpy(&header,bBuf,sizeof(DNSHeader));
	nQuery=ntohs(header.Queries) ? 1 : 0;
	nResp=ntohs(header.AnswerRR) ? 1 : 0;
	if(dataSize<(nQuery*DNSQUERY_SIZE + nResp*DNSANSWER_SIZE))
		throw new Exception("Malformed DNS datagram");

	/* Iterator at the beginning of data (after header) */
	bI=bBuf+sizeof(header);
	if(nQuery){
		/* Find the zero delimiter */
		for(bZ=bI;bZ-bBuf<dataSize;bZ++)	
			if(*bZ==0) break;
		if(bZ-bBuf>=dataSize)
			throw new Exception("Malformed DNS datagram");
		string EncodedHost((char *)bI,bZ-bI);
		string Host;
		DecodeRecord(Host,EncodedHost,0);
		bI=bZ+1;
		query.Record=Host;
		query.Type=*((unsigned short *) bI);
		bI+=sizeof(query.Type);
		query.Class=*((unsigned short *) bI);
		bI+=sizeof(query.Class);	
	}

	if(nResp){

		answer.ID=*((ushort *) bI);
		bI+=sizeof(answer.ID);
		answer.Type=*((ushort *) bI);
		bI+=sizeof(answer.Type);
		answer.Class=*((ushort *) bI);
		bI+=sizeof(answer.Class);
		answer.TTL=*((uint *) bI);
		bI+=sizeof(answer.TTL);
		answer.DataLen=*((ushort *) bI);
		bI+=sizeof(answer.DataLen);
		unsigned short DLen=ntohs(answer.DataLen);
		/* Check if there is enough buffer data to read */

		if(DLen+bI-bBuf>dataSize){
			throw new Exception("Malformed data"); /* Oh wild wild packet */
		}
		//answer.data=Buffer(bI,DLen);
		answer.data.clear();
		answer.data.append(bI,DLen);
		bI+=DLen;
	}	
}

Buffer DNSPacket::Build(){
	Buffer outBuf;

	int nQuery=ntohs(header.Queries) ? 1 : 0;
	int nResp=ntohs(header.AnswerRR) ? 1 : 0;
		
	outBuf.append((byte*)&header,sizeof(header));
	/*Queries*/
	if(nQuery){
		string EncHost;
		GenRecord(EncHost,query.Record,0); //Formats record a.b.c in DNS compliant type (x)a(x)b(x)c
		outBuf.append(EncHost);
		byte zero=0;
		outBuf.append(&zero,1);
		outBuf.append((byte *)&(query.Type),sizeof(query.Type));
		
		outBuf.append((byte *)&(query.Class),sizeof(query.Class));
	}	
	/*Answers*/
	if(nResp){
		int curDataLen=ntohs(answer.DataLen);
		outBuf.append((byte *)&(answer.ID),sizeof(answer.ID));
		outBuf.append((byte *)&(answer.Type),sizeof(answer.Type));
		outBuf.append((byte *)&(answer.Class),sizeof(answer.Class));
		outBuf.append((byte *)&(answer.TTL),sizeof(answer.TTL));
		
		if(answer.Type==TYPE_TXT){
			answer.DataLen=htons(ntohs(answer.DataLen)+1);
			outBuf.append((byte *)&(answer.DataLen),sizeof(answer.DataLen));
			byte bTmp=ntohs(answer.DataLen)-1;
			outBuf.append(&bTmp,1);	
		}else{
			outBuf.append((byte *)&(answer.DataLen),sizeof(answer.DataLen));
		}
		
		
		if(curDataLen > answer.data.size()){
			throw new Exception ("Overflowing DataLength in DNS response");
		}
		
		outBuf.append(answer.data);		
	}	
	
	return outBuf;
}

 void DNSPacket::GenRecord(string &outRecord,string &host,int pos){
 	if(host.substr(host.size()-1,1)!=".")
 		host.append(".");
	string::size_type fdot;
	fdot=host.find('.',pos);
	if(fdot!=string::npos){
		char curDot=char(fdot-pos); 
		outRecord.append(&curDot,1);
		outRecord.append(host.substr(pos,fdot-pos));
		GenRecord(outRecord,host,fdot+1);
	}
 }

void DNSPacket::DecodeRecord(string &outHost,string &record,int pos){
 		int dLen=record.at(pos);
 		if(dLen==0) return;
 		outHost.append(record.substr(pos+1,dLen));
 		if(pos+dLen+1<record.size()){
 		 outHost.append(".",1);
 	   	 DecodeRecord(outHost,record,pos+dLen+1);
 		}
	}


DNSPacket::~DNSPacket(){
}
