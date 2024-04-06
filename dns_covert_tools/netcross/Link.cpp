#include "Link.h"

Link::Link()
{
	LinkID=0;
	estabilished=false;
	
}

Link::~Link()
{
}


void Link::RegisterRecvNotifier(Client& iClient,RecvNotifier* iRNotif){
	NotifyMap[iClient.GetLinkID()]=iRNotif;	
} 

void Link::UnRegisterRecvNotifier(Client& iClient){
	NotifyMap.erase(iClient.GetLinkID());	
} 

void Link::UnRegisterRecvNotifier(unsigned short iID){
	NotifyMap.erase(iID);	
} 
