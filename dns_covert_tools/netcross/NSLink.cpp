#include "NSLink.h"


vector<string> NSLink::GetNeededParams(){
	vector<string> lparam;
	lparam.push_back("dns_server");
	lparam.push_back("host");
	lparam.push_back("bind_ip");
	return lparam;
}
 
NSLink::NSLink()
{	SendingSeq=0;
	LastSeqID=0;
	me=this;
	pthread_mutex_init(&DNSReplyMutex,NULL);
	PollRatio=INIT_POLL_RATIO;
}

NSLink::~NSLink()
{	Terminate();
	pthread_mutex_destroy(&DNSReplyMutex);
	delete sock;
	
}

void NSLink::Init(){

	sock=new Socket(SOCK_DGRAM,0);
	
	if(AppMode==MODE_CLIENT){
		if(Parameters.find("dns_server")==Parameters.end())
			throw new Exception("'dns_server' parameter required");
		if(Parameters.find("host")==Parameters.end())
			throw new Exception("'host' parameter required");
		MaxPacketSize=MAX_QUERY_TOTSIZE-(*Parameters.find("host")).second.size()-5; //era -5
		sock->Connect(Parameters.find("dns_server")->second,53);
		
		
		
	}else{
		if(Parameters.find("bind_ip")==Parameters.end())
			throw new Exception("'bind_ip' parameter required");
		if(Parameters.find("host")==Parameters.end())
			throw new Exception("'host' parameter required");
			
			MaxPacketSize=150; //155 MAX_QUERY_TOTSIZE;	
			sock->BindToIP(53,Parameters.find("bind_ip")->second);	
	}
	
	recThread.Attach(this,&NSLink::RecvThread);
	recThread.Start();
	if(AppMode==MODE_SERVER){
		replyingThread.Attach(this,&NSLink::DNSReplyingThread);
		replyingThread.Start();
	}
	if(AppMode==MODE_CLIENT)
		LinkToServer(); //Estabilish a link with the server
	
}

void NSLink::LinkToServer() {
	Buffer ackBuf(NEWLINK_PREFIX,strlen(NEWLINK_PREFIX));
	Send(ackBuf,0);	
	Buffer recvBuf;

	sleep(LINK_TIMEOUT);

	if(!estabilished)
		throw new Exception("Timeout waiting for an acknowledgment from server");
	
	pollingThread.Attach(this,&NSLink::PollingThread);
	pollingThread.Start();
}


void  NSLink::DNSReplyingThread(){
	
	while(1){
	
		if(DNSReplyQueue.begin()!=DNSReplyQueue.end()){	
			DNSReplyItem &rItem=*(DNSReplyQueue.begin());
			ReplyToClient(rItem.LinkID,rItem.QueryPack,&(rItem.endpoint));
			DNSReplyQueue.erase(DNSReplyQueue.begin());
		}else
		pthread_mutex_lock(&DNSReplyMutex);
	
	}
}


void  NSLink::PollingThread(){
 Buffer polbuf(POLL_PREFIX,strlen(POLL_PREFIX));
 while(1){
 	//usleep(POLL_USEC);
 	sleep(1);
	PollForData(); 	
 }	
}

void NSLink::PollForData(){
 Buffer polbuf(POLL_PREFIX,strlen(POLL_PREFIX));

 if(!sock->IsConnected()) return;
   Send(polbuf,0);
}

void NSLink::Terminate(){
if(AppMode==MODE_CLIENT){
	recThread.Stop();
	pollingThread.Stop();
	UnRegisterRecvNotifier(0);
	 Buffer polbuf(ENDLINK_PREFIX,strlen(ENDLINK_PREFIX));
	 if(sock->IsConnected())
		 Send(polbuf,0);
		 
}   
   sock->close();
}

string NSLink::LinkIDToAscii(){
	ostringstream outStr;
	outStr.width(LINKID_LENGTH);
	outStr.fill('0');
	outStr << right << LinkID;
	return outStr.str();
}

string NSLink::SeqIDToAscii(unsigned short iSeqID){
	ostringstream outStr;
	outStr.width(SEQID_LENGTH);
	outStr.fill('0');
	outStr << right << iSeqID;
	return outStr.str();
}

unsigned short NSLink::AsciiToLinkID(string& idAscii){
	return atoi(idAscii.c_str());
}

void NSLink::Send(Buffer &buf,unsigned short dstLink){


	
	SendingSeq+=0x1000;
	unsigned short LSendingSeq=SendingSeq; //for thread safety
	
	DNSPacket dpack;
	if(AppMode==MODE_CLIENT){
		isSending=true;
		
		bool AppendNSize=false;				
		string record;
		string curBufStr=buf.toString();
		if(curBufStr==NEWLINK_PREFIX)
			record=NEWLINK_PREFIX;
		else if(curBufStr==POLL_PREFIX)
			record=POLL_PREFIX;
		else if(curBufStr==ENDLINK_PREFIX)
			record=ENDLINK_PREFIX;
		else{
			record=buf.Encode(); //Retrieve ASCII-encoded string from buffer	
			AppendNSize=true;
		}

		unsigned short chunks=ceil((float)record.size()/MaxPacketSize);
		if(AppendNSize){
			buf.insert((byte *)&chunks,sizeof(chunks));
			record=buf.Encode();
		}

		
		for(int cI=0;cI<chunks;cI++){
			
			dpack.header.Flags=DNSPacket::FLAGS_QUERY; 
			dpack.header.Queries=htons(1);
			
			//dpack.header.TransID=htons(LSendingSeq+cI); // Transaction ID pseudo-randomly generated
			static unsigned short TransProgID;
			
			dpack.header.TransID=htons(TransProgID++);
			dpack.query.Class=DNSPacket::CLASS_IN;
			dpack.query.Type=DNSPacket::TYPE_TXT;
			dpack.query.Record=record.substr(cI*MaxPacketSize,MaxPacketSize);
			
			string SeqStr;
			if(cI<chunks-1)
				SeqStr =SeqIDToAscii(LSendingSeq+cI);
			else //if it's the last
				SeqStr=SeqIDToAscii(LSendingSeq+0x0fff);
			
			/* Split with dots: aaaabbbbccccdddd -> aaaa.bbbb.cccc.dddd */	

			if(dpack.query.Record!=POLL_PREFIX && dpack.query.Record!=ENDLINK_PREFIX)
				for(int i=MAX_ZONE_LENGTH;i<dpack.query.Record.size();i+=MAX_ZONE_LENGTH)
					dpack.query.Record.insert(dpack.query.Record.begin()+i,'.');
			
			dpack.query.Record.insert(0,SeqStr); //Insert SequenceID in head to manage fragmentation

			if(dpack.query.Record.at(dpack.query.Record.size()-1)!='.')
				dpack.query.Record.append(".");

			dpack.query.Record.append(LinkIDToAscii());
			dpack.query.Record.append(".");
			dpack.query.Record.append(Parameters.find("host")->second);
					
			Buffer sndBuf=dpack.Build();
			sock->SendData(sndBuf);
		}	
		isSending=false;

	}else{ //IF AppMode==MODE_SERVER
		unsigned short chunks=(unsigned short) ceil((float)buf.size()/MaxPacketSize);
		buf.insert((byte *)&chunks,sizeof(chunks));
		for(int cI=0;cI<chunks;cI++){
			dpack.header.Flags=DNSPacket::FLAGS_RESPOK;
			dpack.header.AnswerRR=htons(1);
			dpack.header.Queries=htons(1);

			dpack.header.TransID=cI;

			dpack.query.Type=DNSPacket::TYPE_TXT;
			dpack.query.Class=DNSPacket::CLASS_IN;
			dpack.query.Record.append(POLL_PREFIX);
			dpack.query.Record.append(".");
			dpack.query.Record.append(Parameters.find("host")->second);
				
			dpack.answer.Class=DNSPacket::CLASS_IN;
			dpack.answer.ID=htons(0xc00c);
			dpack.answer.Type=DNSPacket::TYPE_TXT;
			dpack.answer.TTL=0;
			
			string SeqStr;
			if(cI<chunks-1)
				SeqStr=SeqIDToAscii(LSendingSeq+cI);
			else //if it's the last
				SeqStr=SeqIDToAscii(LSendingSeq+0x0fff);
			dpack.answer.data.clear();
			dpack.answer.data.append(SeqStr);
			dpack.answer.data.append(buf.toArray()+cI*MaxPacketSize,(cI==chunks-1) ? buf.size()-cI*MaxPacketSize : MaxPacketSize);
			dpack.answer.DataLen=htons(dpack.answer.data.size());

			txQueue[dstLink].push_back(dpack);
		}		

	}

}

/*----------------------------------------------------------------------------------------------------
 * RecvThread: The receving thread
 * ---------------------------------------------------------------------------------------------------
 */
void NSLink::RecvThread(){
	struct sockaddr_in sender;
	string HostName=Parameters.find("host")->second;
	/*Receive data from socket*/

	while(1){
		Buffer outBuffer;
		unsigned short outLink;
		
		Buffer recvBuf;
		SockRetVal sRes=sock->RecvData(recvBuf,&sender); 

		if(sRes==SOCKRET_TIMEOUT)
			continue;
		else if(sRes!=SOCKRET_OK)
			throw new Exception("Error reading data from socket");
		
		DNSPacket dpack;
		dpack.Parse(recvBuf);
		
		if(AppMode==MODE_SERVER) {
				
				SessionServer &SSrv=SessionServer::GetInstance();
				if(dpack.header.Queries<1) 
					continue; //Invalid packet recived, reject.
				
				int TailLength=HostName.size()+LinkIDToAscii().size()+2;
		
				/* Loop in DNS queries */
				if(dpack.header.Queries){
					
					if(dpack.query.Type!=DNSPacket::TYPE_TXT)
						 continue;
					
						 
					Buffer recBuf;
					/* We are expecting something like this: XXXX..data..XXXXX.nnnnn.host.tld -> Checking for .nnnnn.host.tld  */
		
					if(dpack.query.Record.size()<TailLength)
						continue;

					string tail=dpack.query.Record.substr(dpack.query.Record.size()-TailLength,TailLength);
			
					if(tail.substr(tail.size()-HostName.size(),HostName.size())!=HostName)
						continue;

					string StrLinkID(tail.substr(1,LinkIDToAscii().size()));

					unsigned short RemLinkID=AsciiToLinkID(StrLinkID);
					/* We Got RemLinkID*/
					if(RemLinkID && !SSrv.ClientExists(RemLinkID))
						continue; //We got a request from an unknown client
					outLink=RemLinkID;
					
					string SeqStr=dpack.query.Record.substr(0,SEQID_LENGTH);
					unsigned short SeqID=atoi(SeqStr.c_str());
					
					LastSeqID=SeqID;
					string QueryDataString=dpack.query.Record.substr(SEQID_LENGTH,dpack.query.Record.size()-tail.size()-SEQID_LENGTH);
					////QueryDataString.
								
					bool newlink=(RemLinkID==0) && (QueryDataString==NEWLINK_PREFIX);
					bool polling=QueryDataString==POLL_PREFIX;
					bool endlink=QueryDataString==ENDLINK_PREFIX;
					
					
					if(newlink){
						try{
							unsigned short NewLinkID=SSrv.NewClient(*me);
						 
						 //Send LinkID to client
						 				DNSPacket ackPak;
						 				ackPak.header.Flags=DNSPacket::FLAGS_RESPOK;
										ackPak.header.AnswerRR=htons(1);
										ackPak.header.Queries=htons(1);
										ackPak.header.TransID=dpack.header.TransID;
										ackPak.query.Type=DNSPacket::TYPE_TXT;
										ackPak.query.Class=DNSPacket::CLASS_IN;
										ackPak.query.Record=dpack.query.Record;
										ackPak.answer.Class=DNSPacket::CLASS_IN;
										ackPak.answer.ID=htons(0xc00c);
										ackPak.answer.Type=DNSPacket::TYPE_TXT;
										ackPak.answer.TTL=0;
										
										ostringstream StrLinkID;
										StrLinkID << NewLinkID;
										
										ackPak.answer.data.append((byte*)LINKID_PREFIX,strlen(LINKID_PREFIX));
										ackPak.answer.data.append((byte *)StrLinkID.str().c_str(),StrLinkID.str().size());
										ackPak.answer.DataLen=htons(ackPak.answer.data.size());
							
										//Insert ack in heading and send back to the client
										DEBUG("New client joined link. ID: " << NewLinkID);
										txQueue[NewLinkID].insert(txQueue[NewLinkID].begin(),ackPak); 

										ReplyToClient(NewLinkID,dpack,&sender);
						}catch(Exception *e){
							//Sorry, not enought resources for a new client
							ReplyToClient(0,dpack,&sender); //TODO implementare link 0 nel client
							cout << "No resources to estabilish a link with the client" << endl;
							DEBUG(e->getMessage());
						}
					
					} // END OF: if(newlink)
					/* Remote side is closing the connection */
					else if(endlink){
						DEBUG("Ending link connection with client " << RemLinkID);
						//Flush the Queues
						txQueue.erase(RemLinkID);
						ServerRxQueue.erase(RemLinkID);
						SSrv.RemoveClient(RemLinkID);
						
					}
					/* Check wether client is polling for data or is sending data */
					else if(polling){		
							//The client is polling for data, send him back the transfer queue
							ReplyToClient(RemLinkID,dpack, &sender);	
							//SSrv.ClientAlive(RemLinkID);
							/*DNSReplyItem rpl;
							rpl.LinkID=RemLinkID;
							rpl.QueryPack=dpack;
							rpl.endpoint=sender;
							DNSReplyQueue.insert(DNSReplyQueue.end(),rpl);
							pthread_mutex_unlock(&DNSReplyMutex);*/
					}
					//IF not polling AND not a new link ... -> client is sending data
					else{ 
						
						PurgeDots(QueryDataString);
						
						string MeltData;
						/* SeqID ending with ?FFF are closing transactions */
	
						if((SeqID & 0x0fff)==0x0fff){ 
							unsigned short BaseSeqID=SeqID & 0xf000;
								unsigned short iSeq=BaseSeqID;
							unsigned short NSegm=1;
							unsigned short RSegm;
							for(;iSeq - BaseSeqID<0x1000;iSeq++){
								if((ServerRxQueue[RemLinkID].find(iSeq))==ServerRxQueue[RemLinkID].end()) break;
								MeltData.append(ServerRxQueue[RemLinkID][iSeq]);
								ServerRxQueue[RemLinkID].erase(iSeq);
								NSegm++;
							}
							MeltData.append(QueryDataString);
							outBuffer.Decode(MeltData);
							
							RSegm=*((unsigned char *)outBuffer.toArray());
							
							//Transport the received frame only if the number of segments match
							if(RSegm==NSegm && outBuffer.size()>sizeof(unsigned short)){
								outBuffer.DeleteFirstBytes(sizeof(unsigned short));
								if(NotifyMap.find(RemLinkID)!=NotifyMap.end())
									(*NotifyMap[RemLinkID]).NotifyRecv(outBuffer);
							}else 
							DEBUG("Segment mismatch, dropping (" << RSegm << "  " << NSegm << ")");

						}else{//It it's not a closing transaction append to the rx queue
							ServerRxQueue[RemLinkID][SeqID]=QueryDataString;
						}
						ReplyToClient(RemLinkID,dpack, &sender);
						/*
						DNSReplyItem rpl;
						rpl.LinkID=RemLinkID;
						rpl.QueryPack=dpack;
						rpl.endpoint=sender;
						DNSReplyQueue.insert(DNSReplyQueue.end(),rpl);
						pthread_mutex_unlock(&DNSReplyMutex);
						*/
					}
			}
	
		}else{ //IF mode==MODE_CLIENT

			static unsigned char PollCounter;
			
			if(PollCounter++>=PollRatio && PollRatio<POLL_RATIO_MAX)
				PollCounter=0;
			else{
				PollForData();
			}


			if(dpack.header.AnswerRR<1) 
					continue;

			/* Loop in DNS Answers */
			if(dpack.header.AnswerRR){
				if(dpack.answer.Type!=DNSPacket::TYPE_TXT)
					 continue;
				
				
				if(ntohs(dpack.answer.DataLen)>dpack.answer.data.size() || dpack.answer.data.size()<4)
						continue;
		
				/* Removes the first byte in the TXT response buffer , it contains the content length again (DNS Specs)*/
				dpack.answer.data.DeleteFirstBytes(1);
				string SeqStr=dpack.answer.data.toString().substr(0,SEQID_LENGTH);
				
				/*
				 *  We Got first 5 Bytes of data in SeqStr
				 * They could be the Sequence Number of a packet sequence
				 * or a "LNKID" that's providing a LinkID to client
				 */
	
				if(SeqStr==LINKID_PREFIX){
					LinkID=atoi((char *)dpack.answer.data.toArray()+strlen(LINKID_PREFIX));
					ClientLinkID=LinkID;
					cout << "Linked to the server with ClientID " << LinkID << endl;
					estabilished=true;
					continue;
				}
					
				
				unsigned short SeqID=atoi(SeqStr.c_str()); 
				unsigned short BaseSeqID=SeqID & 0xf000;
				
				dpack.answer.data.DeleteFirstBytes(SEQID_LENGTH);
				ClientRxQueue[SeqID]=dpack.answer.data;

	

				if((SeqID & 0x0fff)==0x0fff){ 
					
					Buffer MeltData;
					unsigned short iSeq=BaseSeqID;
					unsigned short NSegm=1;
					unsigned short RSegm;
				
					for(;iSeq - BaseSeqID<0x1000;iSeq++){ 
						if((ClientRxQueue.find(iSeq))==ClientRxQueue.end()) break;
						MeltData.append(ClientRxQueue[iSeq]);
						ClientRxQueue.erase(iSeq);	
						NSegm++;
					}
					MeltData.append(ClientRxQueue[SeqID]);
					
					RSegm=*((unsigned char *)MeltData.toArray());
					
					if(RSegm==NSegm && MeltData.size()>sizeof(unsigned short)){
						MeltData.DeleteFirstBytes(sizeof(unsigned short));
						
						(*NotifyMap[0]).NotifyRecv(MeltData); //era outBuffe
						ThrottleFeedback(true);
					}else {
						ThrottleFeedback(false); //Send a negative feedback
						DEBUG("Segment mismatch, dropping (" << RSegm << "  " << NSegm << ")");
					}

				}
				
			} //If nAnswer
			
			
		}//END IF mode==MODE_CLIENT

	}//END while(1)

}

void NSLink::PurgeDots(string &inStr){
	
	for(string::iterator i=inStr.begin();i!=inStr.end();i++)
		if(*i=='.') inStr.erase(i);	
}

void NSLink::ReplyToClient(unsigned short RemoteLinkID,DNSPacket& iRequest,struct sockaddr_in *RemoteClient){
	if(RemoteLinkID==0 || txQueue.find(RemoteLinkID)==txQueue.end() || txQueue[RemoteLinkID].size()==0){
		/* If no data in queue for the client send it a blank packet */
		DNSPacket dpack;
		dpack.header.TransID=iRequest.header.TransID;
		dpack.header.Flags=DNSPacket::FLAGS_NOSUCHNAME;
	 	/*Copy the query to generate a valid reply*/
	 	if(iRequest.header.Queries){
	 			dpack.query.Record=iRequest.query.Record;
	 			dpack.query.Class=iRequest.query.Class;
	 			dpack.query.Type=iRequest.query.Type;
	 			dpack.header.Queries=htons(1);
	 	}
		Buffer sndBuf=dpack.Build();
		sock->SendData(sndBuf,RemoteClient);
	}else{
		DNSPacket *qItem=&(txQueue[RemoteLinkID].begin().operator *());
	 	qItem->header.TransID=iRequest.header.TransID;
	 	/*Copy the query to generate a valid reply*/
	 	if(qItem->header.Queries && iRequest.header.Queries)
	 			qItem->query.Record=iRequest.query.Record;

	 	Buffer sndBuf=qItem->Build();
		
	 	sock->SendData(sndBuf ,RemoteClient);
	 	txQueue[RemoteLinkID].erase(txQueue[RemoteLinkID].begin());
	}
}

void NSLink::ThrottleFeedback(bool iFbk){
	static int sValue;
	static unsigned int PositiveFeedbacks;
	if(AppMode==MODE_SERVER) return;
	
	if(iFbk) {
		if(PositiveFeedbacks++>=FEEDBACK_POSITIVE_RATIO){
			 sValue++;
			 PositiveFeedbacks=0;			 
		}
	}
	else
		sValue--;
	if(sValue>=THROTTLE_INCREASE_LIMIT){
		if(PollRatio<POLL_RATIO_MAX) PollRatio++;
		sValue=0;	

	}
	else if(sValue<=THROTTLE_DECREASE_FEEDBACK ){
		if(PollRatio>0) PollRatio--;
		sValue=0;
	}
}
