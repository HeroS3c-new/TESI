#ifndef GLOBALS_H_
#define GLOBALS_H_
#define VERSION "0.9pre-alpha"
extern bool DebugMessages;
extern unsigned short ClientLinkID;
#define DEBUG(str) {if(DebugMessages) cout << str << endl;}
#define SAFE_DELETE(obj) if(obj) {delete obj; obj=NULL;}

#define MODE_CLIENT 0
#define MODE_SERVER 1

using namespace std;

#include <vector>
#include <map>
#include <list>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <net/ethernet.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>

//Forward declarations
class Buffer;
class Exception;
class RecvNotifier;
class ConfParser;
class DNSPacket;
class Link;
class Client;
class Endpoint;
class NSLink;
class RedirectorEndpoing;
class ProxyEndpoint;
class TunTapEndpoint;
class SessionServer;
class LosslessLink;
class TimerCallback;
class Timer;


#include "Buffer.h"
#include "Exception.h"
#include "Thread.h"

#include "RecvNotifier.h"
#include "ConfParser.h"
#include "Socket.h"
#include "DNSPacket.h"
#include "Timer.h"

#include "Link.h"
#include "Client.h"
#include "Endpoint.h"


#include "NSLink.h"
#include "RedirectorEndpoint.h"
#include "TunTapEndpoint.h"
#include "ProxyEndpoint.h"
#include "SessionServer.h"
#include "LosslessLink.h"



#endif /*GLOBALS_H_*/
