#include "Endpoint.h"

Endpoint::Endpoint(Client& iCaller)
{
	refClient=&iCaller;
}


void Endpoint::RegisterRecvNotifier(RecvNotifier *iRNotif){
	RNotif=iRNotif;
}

void Endpoint::Init(){
	if(!RNotif)
	 throw new Exception("No RecvNotfifer provided. Unable to receive data");
}

Endpoint::~Endpoint()

{
}
