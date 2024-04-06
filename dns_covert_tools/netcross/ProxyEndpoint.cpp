#include "ProxyEndpoint.h"

ProxyEndpoint::~ProxyEndpoint()
{
	SAFE_DELETE(sock);
}

vector<string> ProxyEndpoint::GetNeededParams(){
	vector<string> lparam;
	if(AppMode==MODE_CLIENT){
		lparam.push_back("local_port");
		lparam.push_back("bind_ip");
	}
	return lparam;
}

void ProxyEndpoint::Init(){
	Endpoint::Init();

	clientSock=NULL;	
	sock=new Socket(SOCK_STREAM,0);

	if(AppMode==MODE_CLIENT){
		int listenPort;
		if(Parameters.find("local_port")==Parameters.end())
			listenPort=PROXY_DEFAULT_PORT;
		else
			listenPort=atoi(Parameters.find("local_port")->second.c_str());
		
			sock->BindToIP(listenPort,Parameters.find("bind_ip")->second);
			THDRecv.Attach(this,&ProxyEndpoint::ClientRecvThread);
			THDRecv.Start();
	}else{
		if(Parameters.find("bind_ip")!=Parameters.end())
			sock->BindToIP(0,Parameters.find("bind_ip")->second);	
	}
	
}

void ProxyEndpoint::PushBuffer(Buffer& iData){
	static Buffer buf;
	buf.append(iData);

	//Scan for \n\n or \r\n\r\n
	for(unsigned int i=0;i<buf.size();i++){
		if( ! ((buf.At(i)=='\n' && buf.At(i+1)=='\n') || (buf.At(i)=='\r' && buf.At(i+1)=='\n' && buf.At(i+2)=='\r' && buf.At(i+3)=='\n'  )) ) continue;
		byte FByte=CTL_INIT;
		buf.insert(&FByte,1);

		RNotif->NotifyRecv(buf);	
		buf.clear();
		return;
	}
}

bool ProxyEndpoint::ParseHttpRequest(Buffer& iData,string &outHost){
	string bufStr=iData.toString();
	size_t fPos=bufStr.find(" ");
	size_t lPos;
	if(fPos==string::npos) return false;
	if((fPos=bufStr.find("http://",fPos))==string::npos) return false;
	if((lPos=bufStr.find("/",fPos+8))==string::npos) return false;
	outHost=bufStr.substr(fPos+7,lPos-fPos-7);
	bufStr.erase(bufStr.begin()+fPos,bufStr.begin()+lPos);

	return true;
}


void ProxyEndpoint::ClientRecvThread(){
	sock->Listen(1);
	while(1){
		clientSock=sock->Accept();
		DEBUG("Newconn");
		while(1){
			Buffer recBuf;

			try{
				if(clientSock->RecvData(recBuf)!=SOCKRET_OK) 
					throw new Exception("");
				PushBuffer(recBuf);
			}catch(Exception *e){
				byte FByte=CTL_CLOSE;
				Buffer CloseBuf(&FByte,1);
				RNotif->NotifyRecv(CloseBuf);
				SAFE_DELETE(clientSock);	
				break;
				
			}
		}
		
	}	
}

//Receiving thread for the client socket (on the server instance of the program)
void ProxyEndpoint::SrvRecvThread(){
	while(1){
		Buffer recBuf;
		if(!sock) return;
		try{
			if(sock->RecvData(recBuf)!=SOCKRET_OK) throw new Exception("");
			byte FByte=CTL_DATA;
			recBuf.insert(&FByte,1);
			RNotif->NotifyRecv(recBuf);
		}catch(Exception *e){			
			byte FByte=CTL_CLOSE;
			Buffer CloseBuf(&FByte,1);
			RNotif->NotifyRecv(CloseBuf);			
			SAFE_DELETE(sock);

			return;
		}
	}	
}



void ProxyEndpoint::Send(Buffer& recvBuf){
	byte FByte;
	
	/* Analyze the first byte */
	FByte=*(recvBuf.toArray());
	recvBuf.DeleteFirstBytes(1);
	byte EByte=CTL_ERR;
	Buffer ErrBuf(&EByte,1);

	switch(FByte){
		case CTL_INIT:{
			if(AppMode==MODE_CLIENT) return;
			//Only IF AppMode==MODE_SERVER
			string RemoteHost;
			if(!ParseHttpRequest(recvBuf,RemoteHost)){
				RNotif->NotifyRecv(ErrBuf);
				return;	
			}
			SAFE_DELETE(sock);
			sock=new Socket(SOCK_STREAM,0);
			try{
				sock->Connect(RemoteHost,80);
				
			}catch(Exception *e){
				RNotif->NotifyRecv(ErrBuf);
				return;	
			}
			THDRecv.Stop();
			DEBUG("Sending to " << RemoteHost);
			sock->SendData(recvBuf);
			THDRecv.Attach(this,&ProxyEndpoint::SrvRecvThread);
			THDRecv.Start();	
		
			break;}
		case CTL_CLOSE:{
			if(AppMode==MODE_CLIENT) {
				clientSock->close();
					
			}else{//If AppMod==MODE_SERVER
				THDRecv.Stop();
				SAFE_DELETE(sock);
			}
			
			break;	
		}
	   case CTL_ERR:{
	   		if(AppMode==MODE_SERVER) return;
	   		//Only IF AppMode==MODE_CLIENT
	   		if(clientSock->IsConnected()){
	   			Buffer errbuf(ERR_500_STR,strlen(ERR_500_STR));
	   			clientSock->SendData(errbuf);	
	   		}
	   		break;
	   }
	   case CTL_DATA:{
		   	if(AppMode==MODE_SERVER){
				if(!sock) return;
				if(sock->IsConnected()) return;
				sock->SendData(recvBuf);
		   	}else{//IF AppMode==MODE_CLIENT
		   		if(!clientSock->IsConnected()) return;
		   			clientSock->SendData(recvBuf);
		   	}
	   	break;	
	   }
	}
}
