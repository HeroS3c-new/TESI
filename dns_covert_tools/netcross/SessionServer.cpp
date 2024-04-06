#include "SessionServer.h"


SessionServer *SessionServer::singletonObj=NULL;
SessionServer::SessionServer()
{
	THDTimeout.Attach(this,&SessionServer::TimeoutScanThread);
	THDTimeout.Start();
}

SessionServer& SessionServer::GetInstance(){
	if(singletonObj==NULL)
		singletonObj=new SessionServer();
	return *singletonObj;
		
}	

void SessionServer::TimeoutScanThread(){
	while(1){
		TTimeoutMap::iterator iter=TimeoutMap.begin();
		for(;iter!=TimeoutMap.end();iter++)
			if(time(NULL)- (*iter).second > SESSION_TIMEOUT){
				DEBUG("Killing client "<< (*iter).first <<" due to inactivity timeout");	
				 RemoveClient((*iter).first);
			}
		sleep(10);
	}	
}

void SessionServer::ClientAlive(unsigned short iID){
	TimeoutMap[iID]=time(NULL);
}

TClientMap SessionServer::GetClientMap(){
	return clients;
}

unsigned short SessionServer::NewClient(Link& iRefLink){
	unsigned short newID=FindFreeID();
	clients[newID]=new Client(newID,iRefLink);
	return newID;
}	

void SessionServer::RemoveClient(unsigned short iLinkID){
	delete(clients[iLinkID]);
	clients.erase(iLinkID);	
	TimeoutMap.erase(iLinkID);
}

bool SessionServer::ClientExists(unsigned short iLinkID){
	return clients.find(iLinkID)!=clients.end();	
}

Client& SessionServer::GetClient(unsigned short iLinkID){
	if(!ClientExists(iLinkID))
		throw new Exception("Cannot find client");
	return *(clients[iLinkID]);	
}

unsigned short SessionServer::FindFreeID(){
	unsigned short FreeID;
	static unsigned short LastAssignedID;
	TClientMap::iterator iter=clients.begin();
	//Try to allocate freshen ID first (to avoid DNS re-queries)
	for(FreeID=LastAssignedID+1;FreeID < std::numeric_limits<unsigned short>::max() ;FreeID++)
		if(clients.find(FreeID)==clients.end()) {
			LastAssignedID=FreeID;
			return FreeID;
		}
	
	//If not, rescan from zero
	for(FreeID=0;FreeID < LastAssignedID ;FreeID++)
		if(clients.find(FreeID)==clients.end()) {
			LastAssignedID=FreeID;
			return FreeID;
		}
	
		
		
	// If all slots are taken try to recycle never authenticated connections
	 for(FreeID=1;FreeID < std::numeric_limits<unsigned short>::max() ;FreeID++){
	 	if( ! clients[FreeID]->GetAuthStatus()){
	 		clients.erase(FreeID); //Sorry :-/
	 		LastAssignedID=FreeID;
	 		return FreeID;
	 	}
	 }
	
	throw new Exception("Maximum limit of clients reached");
}	


SessionServer::~SessionServer()
{
}
