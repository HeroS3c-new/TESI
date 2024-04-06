#include "globals.h"
#ifndef TIMER_H_
#define TIMER_H_

#define NOEVENTS_SLEEP_USEC 500000
#define MAX_SLEEP_USEC 2000000

class TimerCallback {
 friend class Timer;
 private:
 	virtual void TimerEvent(unsigned int)=0;	
};

class Timer
{
private:
     TimerCallback *CallbackPtr;
     pthread_mutex_t mapMutex;
     map<unsigned int,timeval> timMap;
     void CounterThread();
     Thread<Timer> thdCounter;
     bool IsExpired(unsigned int);
	 unsigned long GetExpirationUSecs(unsigned int);     
public:
	Timer();
	virtual ~Timer();
   	void Attach(TimerCallback *);	
	void AddNewTimer(unsigned int,unsigned short);
	void KillTimer(unsigned int);
};

#endif /*TIMER_H_*/
