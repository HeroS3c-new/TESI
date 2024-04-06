#include "LosslessLink.h"

LosslessLink::LosslessLink(Link& iLink)
{
	baseLink=&iLink;
	baseLink->me=this;
	wSize=LOSSLESS_INIT_WSIZE;
	timerObj.Attach(this);
}

LosslessLink::~LosslessLink()
{
	delete baseLink;
}

void LosslessLink::Init()
{
	baseLink->Parameters=Parameters;
	baseLink->Init();
}

void LosslessLink::Terminate()
{
	baseLink->Terminate();
}

vector<string>  LosslessLink::GetNeededParams()
{
	return baseLink->GetNeededParams();
}
	
void LosslessLink::LockLinkMutex(unsigned short iLinkID,bool initOnly=false){

	bool isInit=false;
	
	if(MutexMap.find(iLinkID)==MutexMap.end()){
		pthread_mutex_init(&(MutexMap[iLinkID]),NULL);
		isInit=true;
	}

	if(!initOnly || (initOnly && isInit )){
		SedingLocked=true;	
		timespec TStimeout;
		timeval TVtimeout;
		gettimeofday(&TVtimeout,NULL);
		TStimeout.tv_nsec=TVtimeout.tv_usec;
		TStimeout.tv_sec=TVtimeout.tv_sec + LOSSLESS_MAX_TIMEOUTS;

		if(pthread_mutex_timedlock(&(MutexMap[iLinkID]),&TStimeout)){
			pthread_mutex_unlock(&(MutexMap[iLinkID]));
		}
	
		
	}

}

void LosslessLink::UnlockLinkMutex(unsigned short iLinkID){
	if(!SedingLocked) return;
	pthread_mutex_unlock(&(MutexMap[iLinkID]));
	SedingLocked=false;
}

	
void LosslessLink::Send(Buffer& iBuf,unsigned short iLinkID){

	LockLinkMutex(iLinkID,true);

	if(txBuf[iLinkID].size()>wSize){
		LockLinkMutex(iLinkID);
	}
	
		unsigned short newID=++lastSent[iLinkID];
		txBuf[iLinkID].insert(txBuf[iLinkID].end(),pair<unsigned short,Buffer>(newID,iBuf));
		timerObj.AddNewTimer(GetTimerID(iLinkID,newID),LOSSLESS_ACK_TIMEOUT);
		SendToBase(iBuf,iLinkID,newID,CTL_Send);
}

unsigned int LosslessLink::GetTimerID(unsigned short iLinkID, unsigned short iSeqID){
	unsigned int ret;
	ret=iLinkID;
	ret=ret << 16;
	ret=ret | iSeqID;
	return ret;
}

void LosslessLink::GetParamsFromTimer(unsigned int iTimID, unsigned short &oLinkID,unsigned short &oSeqID){
	oSeqID=iTimID & 0x0000FFFF;
	oLinkID=iTimID >> 16;
}

void LosslessLink::TimerEvent(unsigned int iTimID){
	unsigned short LinkID, SeqID;
	GetParamsFromTimer(iTimID,LinkID,SeqID);
	TimeoutsMap[iTimID]+=1;
	if(TimeoutsMap[iTimID]>=LOSSLESS_MAX_TIMEOUTS){
		DEBUG("Max restransmission tries, dropping");
		baseLink->ThrottleFeedback(false);
		TimeoutsMap.erase(iTimID);
		return;	
	}
	if(txBuf[LinkID].find(SeqID)==txBuf[LinkID].end()){

		return;
	}
	timerObj.AddNewTimer(iTimID,LOSSLESS_ACK_TIMEOUT);
	SendToBase(txBuf[LinkID][SeqID],LinkID,SeqID,CTL_Send);
	DEBUG("Resending " << SeqID << " size " << txBuf[LinkID][SeqID].size());
}


void LosslessLink::SendToBase(Buffer& iBuf,unsigned short iLinkID,unsigned short SeqID,byte iHeader){
	Buffer sndBuf;
	//Append 3 bytes: FrameSendingControl[1] + FrameSequenceID[2]
	sndBuf.append(&iHeader,1);
	sndBuf.append((byte *)&SeqID,2);
	sndBuf.append(iBuf);
	baseLink->Send(sndBuf,iLinkID);

}


//For server callback
void LosslessLink::RegisterRecvNotifier(Client& iClient,RecvNotifier* iNotif){
	DEBUG("Registered notifier through lossless link");
	Link::RegisterRecvNotifier(iClient,iNotif);
	
	MiddleNotifyMap[iClient.GetLinkID()].baseNotifier=NotifyMap[iClient.GetLinkID()];
	MiddleNotifyMap[iClient.GetLinkID()].baseLink=baseLink;
	MiddleNotifyMap[iClient.GetLinkID()].losLink=this;
	MiddleNotifyMap[iClient.GetLinkID()].refClient=&iClient;

	baseLink->RegisterRecvNotifier(iClient,&(MiddleNotifyMap[iClient.GetLinkID()]));
}

void LosslessLink::UnRegisterRecvNotifier(Client& iClient){
	Link::UnRegisterRecvNotifier(iClient);
	MiddleNotifyMap.erase(iClient.GetLinkID());
	baseLink->UnRegisterRecvNotifier(iClient);
}

void LosslessLink::UnRegisterRecvNotifier(unsigned short iID){
	Link::UnRegisterRecvNotifier(iID);
	MiddleNotifyMap.erase(iID);
	baseLink->UnRegisterRecvNotifier(iID);
}


/*------------------------------------------------------------------------------------------------
 * LosslessReceiver class
 * -----------------------------------------------------------------------------------------------
 */
void LosslessReceiver::NotifyRecv(Buffer& iBuf){
	byte FByte;
	unsigned short curSeq;
	unsigned short LastRecvID;
	unsigned short LinkID=refClient->GetLinkID();

	Buffer blankBuf;
	if(iBuf.size()<3) return;
	FByte=*(iBuf.toArray());
    curSeq=*((unsigned short *)(iBuf.toArray()+1));
	
	TBufferMap::iterator iter;
	iBuf.DeleteFirstBytes(3); //flush overhead bytes
    LastRecvID=losLink->lastRecv[LinkID];


	switch(FByte){
		case LosslessLink::CTL_Send:
		case LosslessLink::CTL_Lost:{
			//In case of restrasmission only notify  the remote part that we've received the frame
			if(curSeq<=LastRecvID && LastRecvID!=0xffff && LastRecvID!=0){
				 losLink->SendToBase(blankBuf,LinkID,curSeq,LosslessLink::CTL_Recvd);
				  DEBUG("Got a duplicate frame, discarding");
 				 losLink->baseLink->ThrottleFeedback(false);
			}
			else {
				losLink->rxBuf[LinkID][curSeq]=iBuf;
				//If received frame is not contiguous
				if((curSeq!=0 && curSeq>LastRecvID+1) || (curSeq==0 && LastRecvID!=0xffff)){
					
					unsigned short LostNum=LastRecvID+1;
				 	Buffer LostBuf((byte *) &LostNum,2);
				 	DEBUG("Lost frame no " << LostNum);
				 	losLink->SendToBase(LostBuf,LinkID,curSeq,LosslessLink::CTL_RecvAndLost);
				}
				else{
					//Scan for contiguous frames in the RxBuf	
					 while(losLink->rxBuf[LinkID].find(LastRecvID+1)!=losLink->rxBuf[LinkID].end()){
						 losLink->lastRecv[LinkID]=++LastRecvID;
						 baseNotifier->NotifyRecv(losLink->rxBuf[LinkID][LastRecvID]);
						 losLink->rxBuf[LinkID].erase(LastRecvID);
					 }
					 Buffer ackBuf((byte *)&LastRecvID,2);

					 losLink->SendToBase(ackBuf,LinkID,curSeq,LosslessLink::CTL_RecvAndACK);
				}
			}
			if(FByte==LosslessLink::CTL_Lost){
				DEBUG("A data loss occourred");
				losLink->baseLink->ThrottleFeedback(false);
			}
			break;}
		
		case LosslessLink::CTL_RecvAndACK:
		case LosslessLink::CTL_RecvAndLost:
		case LosslessLink::CTL_Recvd:{

			if(iBuf.size()<2 && FByte!=LosslessLink::CTL_Recvd) {
				DEBUG("Malformed packet");
				break;	
			}
			
			losLink->timerObj.KillTimer(losLink->GetTimerID(LinkID,curSeq)); 			
			
			if(losLink->txBuf[LinkID].begin()==losLink->txBuf[LinkID].end()){
				break;	
			}

			if(FByte==LosslessLink::CTL_Recvd)
				return;
			
			unsigned short RID=*((unsigned short *)iBuf.toArray());
				
			if(FByte==LosslessLink::CTL_RecvAndACK){
				losLink->baseLink->ThrottleFeedback(true);
				unsigned short TxID=losLink->txBuf[LinkID].begin()->first;


				for(;TxID!=RID;TxID++){
					losLink->timerObj.KillTimer(losLink->GetTimerID(LinkID,TxID)); 
					losLink->txBuf[LinkID].erase(TxID);
				}
				losLink->UnlockLinkMutex(LinkID);
			}else {//IF CTL_RecvAndLost
				losLink->baseLink->ThrottleFeedback(false);
				if(losLink->txBuf[LinkID].find(RID)==losLink->txBuf[LinkID].end()){
					//Sorry,we don't have the sent frame in buffer anymore
					DEBUG("A Frame was irremediably lost");
					losLink->SendToBase(blankBuf,LinkID,RID,LosslessLink::CTL_Lost);
					losLink->timerObj.KillTimer(losLink->GetTimerID(LinkID,	RID)); 

					break; 	
				}
			
				Buffer &resBuf=losLink->txBuf[LinkID][RID];
				losLink->timerObj.KillTimer(losLink->GetTimerID(LinkID,RID)); 
				losLink->SendToBase(resBuf,LinkID,RID,LosslessLink::CTL_Send);
				losLink->timerObj.AddNewTimer(losLink->GetTimerID(LinkID,RID),LOSSLESS_ACK_TIMEOUT);
			}			
			break;}			
		default:{
			DEBUG("Malformed packet");
			losLink->SendToBase(blankBuf,LinkID,LastRecvID+1,LosslessLink::CTL_Lost);
			break;}
	}
}

