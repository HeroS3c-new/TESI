#include "TunTapEndpoint.h"

int TunTapEndpoint::SfdTap;
int TunTapEndpoint::tapType;
Thread<TunTapEndpoint> TunTapEndpoint::STHDRec;
string TunTapEndpoint::StapDev;




TunTapEndpoint::~TunTapEndpoint()
{
	if(tapType!=IFF_TAP){
		IfScript(TUNTAP_IFSCRIPT_DOWN);
		close(*fdTap);
		delete fdTap;
		delete THDRec;
		delete tapDev;	
	}

}

vector<string> TunTapEndpoint::GetNeededParams(){
	vector<string> lparam;
	lparam.push_back("type");
	lparam.push_back("ifup_script");
	lparam.push_back("ifdown_script");
	return lparam;
}

void TunTapEndpoint::OpenInterface(){
   struct ifreq ifr;


   if( (*fdTap = open(TUNTAPDEV, O_RDWR)) < 0 )
     throw new Exception("Cannot open Tun/Tap device");

   memset(&ifr, 0, sizeof(ifr));

   ifr.ifr_flags = tapType;

   if( ioctl(*fdTap, TUNSETIFF, (void *) &ifr) < 0 )
     throw new Exception("Error on ioctl()");
   
     
   *tapDev=ifr.ifr_name;
   DEBUG("Tun/Tap device is " << *tapDev);
   IfScript(TUNTAP_IFSCRIPT_UP);
}

void TunTapEndpoint::IfScript(char iUp){
   string param=iUp==TUNTAP_IFSCRIPT_UP ? "ifup_script" : "ifdown_script";
   string spath;
   if(Parameters.find(param)==Parameters.end())
   	return;
   	
   spath=Parameters.find(param)->second;
   if(spath=="")
    return;

	if(!refClient)
	  return;

   	ostringstream IDStr;
   	if(AppMode==MODE_SERVER) 
		IDStr << refClient->GetLinkID();
	else
		IDStr << ClientLinkID;
   spath.append(" ");
   spath.append(AppMode==MODE_CLIENT ? "cli" : "srv");
   spath.append(" ");
   spath.append(*tapDev);
   spath.append(" ");
   spath.append(IDStr.str());   
   
   DEBUG("launching " << spath);
   if(system(spath.c_str())==-1){
   	throw new Exception("Could not execute the IFace script");
   }

}

bool TunTapEndpoint::BroadcastClients(){
	return tapType==IFF_TAP;	
}
void TunTapEndpoint::Init(){
	if(tapType==IFF_TAP && SfdTap) //Tap endpoint needs to be initialized only once
		return;
		
	Endpoint::Init();
	if(Parameters.find("type")==Parameters.end())
		tapType=IFF_TAP;
	else
		tapType=(Parameters.find("type")->second=="tun") ? IFF_TUN : IFF_TAP;

	if(tapType==IFF_TAP){
		fdTap=&SfdTap;
		THDRec=&STHDRec;
		tapDev=&StapDev;
	}else{
		fdTap=new int();
		THDRec=new Thread<TunTapEndpoint>();
		tapDev=new string();
	}
	
	
	OpenInterface();
	THDRec->Attach(this,&TunTapEndpoint::RecvThread);
	THDRec->Start();
}

void TunTapEndpoint::Send(Buffer &recvBuf){
	int res;
	if(recvBuf.size()<=0) return;
	res=write(*fdTap,recvBuf.toArray(),recvBuf.size());
	if(res<0){
		DEBUG(strerror(errno));
		throw new Exception("Error writing to device");
	}
}


void TunTapEndpoint::RecvThread(){
	DEBUG("Tun tap recv thread : " << hex << pthread_self());
	Buffer recvBuf;	
	unsigned short chunkSize=TUNTAP_RSIZE;
	ssize_t res;
	while(1){

		Buffer chunkBuf;
		chunkBuf.allocate(chunkSize);
		
		if((res=read(*fdTap,chunkBuf.toArray(),chunkSize))<0) //TODO: il doppio passaggio di buffer si puo evitare?
						throw new Exception(strerror(errno));
		
		recvBuf.clear();
		recvBuf.append(chunkBuf.toArray(),res);

		RNotif->NotifyRecv(recvBuf);

	}	
}

