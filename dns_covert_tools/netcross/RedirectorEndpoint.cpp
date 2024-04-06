#include "RedirectorEndpoint.h"


RedirectorEndpoint::~RedirectorEndpoint()
{
	if(sock)
		delete sock;
}


vector<string> RedirectorEndpoint::GetNeededParams(){
	vector<string> lparam;
	if(AppMode==MODE_CLIENT){
		lparam.push_back("local_port");
		lparam.push_back("remote_host");
		lparam.push_back("remote_port");
		lparam.push_back("bind_ip");
	}
	return lparam;
}

void RedirectorEndpoint::Init(){
	Endpoint::Init();
	connected=false;
		
	sock=new Socket(SOCK_STREAM,0);
		
	if(AppMode==MODE_CLIENT){


		if(Parameters.find("local_port")==Parameters.end())
			throw new Exception("'local_port' parameter required");
		if(Parameters.find("remote_host")==Parameters.end())
			throw new Exception("'remote_host' parameter required");
		if(Parameters.find("remote_port")==Parameters.end())
			throw new Exception("'remote_port' parameter required");

		sock->BindToIP(atoi(Parameters.find("local_port")->second.c_str()),"0.0.0.0");

		SendCliParamsToServer();
		//TODO:elimina WaitIncomingConnection();
		
	}else{
		if(Parameters.find("bind_ip")!=Parameters.end())
			sock->BindToIP(0,Parameters.find("bind_ip")->second);	
	}
	
}


void RedirectorEndpoint::WaitIncomingConnection(){
	sock->WaitForConnection()	;
	DEBUG("A new connection has arrived on the endpoint");
	SendState(CNT_Newconn); //Notifiy server that a new connection is arrived on the endpoint
}



void RedirectorEndpoint::SendCliParamsToServer(){
		Buffer sndBuf;
		unsigned char FByte=(unsigned char)CNT_Params;
		sndBuf.append(&FByte,1);
		sndBuf.append(Parameters.find("remote_host")->second);
		FByte=';';
		sndBuf.append(&FByte,1);
		sndBuf.append(Parameters.find("remote_port")->second);	
		RNotif->NotifyRecv(sndBuf);
}

void RedirectorEndpoint::SendState(unsigned char iState){
	Buffer sndBuf;
	unsigned char FByte=iState;
	sndBuf.append(&FByte,1);
	RNotif->NotifyRecv(sndBuf);
}


void RedirectorEndpoint::SendError(string msg){
	Buffer sndBuf;
	unsigned char FByte=CNT_Error;
	sndBuf.append(&FByte,1);
	sndBuf.append(msg);
	RNotif->NotifyRecv(sndBuf);
	//refLink->Send(sndBuf,refClient->GetLinkID());	
}

/* Send Data received from remote link to the endpoint*/
void RedirectorEndpoint::Send(Buffer &recvBuf){
	unsigned char FByte;
	
	string errstr;
	/* Analyze the first byte */
	FByte=*(recvBuf.toArray());
	recvBuf.DeleteFirstBytes(1);

	switch(FByte){
		case CNT_Params:
				if(AppMode==MODE_CLIENT) return; //parameters only allowed from client to server
			
				try{
					string sData=recvBuf.toString();
					size_t vPos=sData.find(";");
					if(vPos==string::npos) 
						throw new Exception("Malformed data");
					remoteHost=sData.substr(0,vPos);

					remotePort=atoi(sData.substr(vPos+1).c_str());
					SendState(CNT_Ready);
				}catch(Exception *e){
					SendError(e->getMessage());	
				}
			break;
		case CNT_Newconn:
			try{
				if(AppMode==MODE_CLIENT) return;
				DEBUG("Connecting socket to " << remoteHost << " on port " << remotePort);
				sock->Connect(remoteHost,remotePort);

				connected=true;

				THDRecv.Attach(this,&RedirectorEndpoint::RecvThread);
				THDRecv.Start();
				SendState(CNT_Connready);
			}catch(Exception *e){
					SendError(e->getMessage());	
			}
			
			break;			
		case CNT_Connready:
			if(AppMode==MODE_SERVER) return;		
			DEBUG("Server endpoint ready");
			clientSock=sock->Accept(); //TODO: occhio ai delete qui
			connected=true;
			sock->Listen(1);
			DEBUG("Accepted client connection");
			THDRecv.Attach(this,&RedirectorEndpoint::RecvThread);
			THDRecv.Start();

			break;
		case CNT_Ready:
			if(AppMode==MODE_SERVER) return;
			DEBUG("Server ready");
			sock->Listen(1);
			THDAcc.Attach(this,&RedirectorEndpoint::WaitIncomingConnection);
			THDAcc.Start();
			//WaitIncomingConnection();
			break;
		case CNT_Error:
			errstr.append("Server raised an error: ");
			errstr.append(recvBuf.toString());
			DEBUG(errstr);
			throw new Exception(errstr);
			break;
		case CNT_Data:
				//TODO:sistemare qui
			if(AppMode==MODE_SERVER) {
				if(sock->IsConnected())
					sock->SendData(recvBuf);
			}else{
				if(clientSock)
					clientSock->SendData(recvBuf);
			}
			break;
		case CNT_Close:
			DEBUG("Remote said: Connection closed");
			THDRecv.Stop();
			if(AppMode==MODE_SERVER){
				sock->close();
			}else{ //AppMode==MODE_CLIENT
				 //Stop the receiving thread
				if(clientSock) clientSock->close();

				//WaitIncomingConnection(); //Wait for new connections
				THDAcc.Stop();
				THDAcc.Attach(this,&RedirectorEndpoint::WaitIncomingConnection);
				THDAcc.Start();
			}
			break;
	}
}

bool RedirectorEndpoint::IsConnected(){
	return connected;	
}

void RedirectorEndpoint::RecvThread(){
	DEBUG("Endpoint receiving thread started");
	Buffer sockrecbuf;
	Socket *whatSock=(AppMode==MODE_SERVER) ? sock : clientSock;
	try{	

	while(1){
		if(!whatSock) {
			DEBUG("No Sock object");
			return;}
		if(!whatSock->IsConnected()) {
			DEBUG("My endpoint connection closed");
			SendState(CNT_Close);
			connected=false; 
			if(AppMode==MODE_CLIENT)
				WaitIncomingConnection();
			return;
		}

		int res=whatSock->RecvData(sockrecbuf);
		if(res==SOCKRET_CLOSINGSOCK){
			DEBUG("My endpoint connection closed");
			SendState(CNT_Close);
			connected=false;
			if(AppMode==MODE_CLIENT)
				WaitIncomingConnection();
			return;
		}


		Buffer retBuf;
		byte fb=CNT_Data;
		retBuf.append(&fb,1);
		retBuf.append(sockrecbuf);
		RNotif->NotifyRecv(retBuf);
	}
	}catch(Exception *e){
		DEBUG("Exception in recv : " << e->getMessage());	
	}

}

