#include "Socket.h"

Socket::Socket(int iType=SOCK_STREAM,int iProto=0)
{
	constructor(iType,iProto);
}

void Socket::constructor(int iType,int iProto=0)
{
	int on=1;
	if((sock=socket(AF_INET,iType,iProto))<0)
		throw new Exception("Cannot create socket");
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	type=iType;
	opened=false;
	OperTimeout=0;
}

bool Socket::IsConnected()
{
	struct sockaddr_in saddr;
	socklen_t readlen=sizeof(saddr);
	bool sockstatus=(getpeername(sock,(struct sockaddr*)&saddr,&readlen)==0);
	if(type==SOCK_STREAM)
		return sockstatus ;
	else
		return opened;
}

void Socket::BindToIP(int port,string ipaddr){
	struct sockaddr_in saddr;
	memset(&saddr,0,sizeof(saddr));
	saddr.sin_family=AF_INET;
	saddr.sin_port=htons(port);
	saddr.sin_addr.s_addr=inet_addr(ipaddr.c_str());
	
	
	if(bind(sock,(struct sockaddr *)&saddr,sizeof(saddr)))
		throw new Exception("Cannot bind socket");
}

void Socket::Connect(string dstaddr,int port){
	memset(&remoteEP,0,sizeof(remoteEP));
	remoteEP.sin_family=AF_INET;
	remoteEP.sin_port=htons(port);
	remoteEP.sin_addr.s_addr=inet_addr(dstaddr.c_str());
	//If it's not a valid ip try to resolve hostname
	if(remoteEP.sin_addr.s_addr==INADDR_NONE){
		hostent *he;
		if ((he = gethostbyname(dstaddr.c_str()) )== NULL) 
			throw new Exception("Cannot resolve host");
	  memcpy(&remoteEP.sin_addr, he->h_addr_list[0], he->h_length);		
	}
	
	if(IsConnected()){
		close();
	}
	if(type==SOCK_STREAM){
		if(connect(sock,(struct sockaddr *)&remoteEP,sizeof(remoteEP))<0){
			throw new Exception(strerror(errno));}	
	}
	opened=true;
}

void Socket::Listen(int maxConn){
	if(type!=SOCK_STREAM) return;
	if(listen(sock,maxConn))
		throw new Exception(strerror(errno));
}

Socket * Socket::Accept(){
	if(type!=SOCK_STREAM)
		throw new Exception("Accept not allowed on non-stream socket");
	struct sockaddr_in remoteConn;
	socklen_t remoteLen=sizeof(remoteConn);
	int newsock;
	Socket *retSock;
	if(newsock=accept(sock,(struct sockaddr *)&remoteConn,&remoteLen)){
		retSock=new Socket(type);
		retSock->sock=newsock;
		retSock->opened=true;
		retSock->remoteEP=remoteConn;
		retSock->OperTimeout=0;
		retSock->type=type;
		return retSock;	
	}else
	throw new Exception(strerror(errno));
}

void Socket::WaitForConnection(){
	struct pollfd fdPoll;
	int nPoll=1;
	fdPoll.fd=sock;
	fdPoll.events=POLLRDNORM;
	if(poll(&fdPoll,nPoll,-1)<=0){
		throw new Exception("Error on poll() : " + string(strerror(errno)));	
	}

	return;
}


unsigned long Socket::SendData(Buffer &data){
	return SendData(data,NULL);
}

unsigned long Socket::SendData(Buffer &data,struct sockaddr_in *RemoteClient){
	long res;
	if(! RemoteClient) RemoteClient=&remoteEP;
	if(type==SOCK_STREAM)
		res=send(sock,data.toArray(),data.size(),0);
	else
		res=sendto(sock,data.toArray(),data.size(),0,(struct sockaddr *)RemoteClient,sizeof(*RemoteClient));

	if(res<0)
		throw new Exception("send() failed");
	else
		return res;	

}


SockRetVal Socket::RecvData(Buffer& inData,struct sockaddr_in *SenderEP){
	ssize_t res;
	socklen_t len=sizeof(remoteEP);
	fd_set fd_recv;
	struct timeval fd_timeout;

	if(OperTimeout>0){
			fd_timeout.tv_sec=OperTimeout;
			FD_ZERO(&fd_recv);
			FD_SET(sock,&fd_recv);
			res=select(sock+1,&fd_recv,(fd_set *)0,(fd_set *)0,&fd_timeout);
			FD_CLR(sock,&fd_recv);
			if(res<0)
				throw new Exception("Error on select(): "+ string(strerror(errno)));
			if(res==0)
			return SOCKRET_TIMEOUT;
	}

	inData.clear();
	inData.allocate(SOCK_BUF_SIZE);
	if(type==SOCK_STREAM){
		res=recv(sock,inData.toArray(),SOCK_BUF_SIZE,0);
		
	}else if(type==SOCK_DGRAM){
		res=recvfrom(sock,inData.toArray(),SOCK_BUF_SIZE,0,(sockaddr *) SenderEP, SenderEP ? &len : NULL);
	}

	if(res<0)
			throw new Exception("recv() failed " + string(strerror(errno)));
	else if(res==0){
			opened=false;
			return SOCKRET_CLOSINGSOCK;
	}
	inData.append(inData.toArray(),res);

	return SOCKRET_OK;
}

void Socket::SetOperationTimeout(unsigned short iSecs){
	OperTimeout=iSecs;
}

SockRetVal Socket::RecvData(Buffer& inData){
	struct sockaddr_in saddrany;
	memset(&saddrany,0,sizeof(saddrany));
	saddrany.sin_family=AF_INET;
	saddrany.sin_addr.s_addr=INADDR_ANY;
	return RecvData(inData,&saddrany);
}

void Socket::close(){
	if(sock){
		if(IsConnected())
			shutdown(sock,SHUT_RDWR);
		::close(sock);
	}
	opened=false;

	constructor(type);
}
int Socket::getSockFD(){
	return sock;
}

Socket::~Socket()
{
}
