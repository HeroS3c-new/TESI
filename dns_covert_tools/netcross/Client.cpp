
#include "Client.h"

Client::Client(unsigned short iLink,Link& iRefLink)
{
	authenticated=false;
	LinkID=iLink;
	ReferringLink=&iRefLink;
	ReferringEndpoint=CreateEndpoint();
	//Register the receiving notifiers
	EndpointReceiver.clientObj=this;
	LinkReceiver.clientObj=this;
	ReferringEndpoint->RegisterRecvNotifier(&EndpointReceiver);

	ReferringLink->RegisterRecvNotifier(*this,&LinkReceiver);
	ReferringEndpoint->Init();
}

unsigned short Client::GetLinkID()
{
	return LinkID;
}

Endpoint * Client::CreateEndpoint(){
	string sTmp;
	Endpoint *retep;
	
	sTmp=cp.getVar("global","endpoint");
	if(sTmp=="redirector")
		retep=new RedirectorEndpoint(*this);
	else if(sTmp=="tuntap")
		retep=new TunTapEndpoint(*this);
	else if(sTmp=="httpproxy")
		retep=new ProxyEndpoint(*this);
	else{
		cout << "Unsupported endpoint. Exiting" << endl;
		exit(1);
	}
	vector<string> nparams=retep->GetNeededParams();

	for(unsigned int i=0;i<nparams.size();i++){
		sTmp=cp.getVar("endpoint",nparams[i]);
		if(sTmp=="" && AppMode==MODE_CLIENT)
		{
			cout << "Insert parameter " << 	nparams[i] << " : " ;
			cin >> sTmp;
			cout << endl;	
		}
		retep->Parameters[nparams[i]]=sTmp;
	}	
	return retep;
}

Link& Client::GetLinkObj()
{
	return *ReferringLink;
}

Endpoint& Client::GetEndpointObj()
{
	if(ReferringEndpoint)
		return *ReferringEndpoint;
	else	
		throw new Exception("No endpoint associated to this client");
}

void Client::CLinkReceiver::NotifyRecv(Buffer &iData)
{
	SessionServer::GetInstance().ClientAlive(clientObj->LinkID);
	clientObj->ReferringEndpoint->Send(iData);
}
void Client::CEndpointReceiver::NotifyRecv(Buffer &iData)
{

	SessionServer::GetInstance().ClientAlive(clientObj->LinkID);
	if(AppMode==MODE_SERVER && clientObj->GetEndpointObj().BroadcastClients()){
		TClientMap cliMap=SessionServer::GetInstance().GetClientMap();
		TClientMap::iterator iter=cliMap.begin();
		while(iter!=cliMap.end()){
			(*iter).second->ReferringLink->Send(iData,(*iter).second->GetLinkID());
			iter++;	
		}
	}
	else
	clientObj->ReferringLink->Send(iData,clientObj->GetLinkID());
}

bool Client::GetAuthStatus()
{
	return authenticated;
}

void Client::SetAuthStatus(bool status)
{
	authenticated=status;
}


Client::~Client()
{	ReferringLink->UnRegisterRecvNotifier(*this);
	delete ReferringEndpoint;

}
