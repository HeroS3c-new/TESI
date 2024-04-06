#include "Timer.h"

Timer::Timer(){
	pthread_mutex_init(&mapMutex,NULL);	
}

Timer::~Timer(){
	thdCounter.Stop();
	pthread_mutex_destroy(&mapMutex);
}	

void Timer::Attach(TimerCallback *iCallback){
	 CallbackPtr=iCallback;
	 
	 thdCounter.Attach(this,&Timer::CounterThread);
	 thdCounter.Start();
}

void Timer::CounterThread(){
	unsigned long usecs;
	map<unsigned int,timeval>::iterator iter,tmpiter;
	
	while(1){
		usecs=NOEVENTS_SLEEP_USEC;
		pthread_mutex_lock(&mapMutex);
		iter=timMap.begin();
		while(iter!=timMap.end()){
			unsigned int TimID=iter->first;
			if(IsExpired(TimID)){
				tmpiter=iter;
				iter++;
				timMap.erase(tmpiter);
				pthread_mutex_unlock(&mapMutex);
				CallbackPtr->TimerEvent(TimID);	

				pthread_mutex_lock(&mapMutex);
			}else{
				usecs=GetExpirationUSecs(TimID);
				break;
			}
			if(iter==timMap.end()) break;
			iter++;
		}
		
		pthread_mutex_unlock(&mapMutex);
		usleep(usecs);	
	}
	
}

unsigned long Timer::GetExpirationUSecs(unsigned int iID){
	map<unsigned int,timeval>::iterator iter;
	unsigned long res;
	iter=timMap.find(iID);
	if(iter==timMap.end())
		return NOEVENTS_SLEEP_USEC;
	
	timeval tevent=timMap[iID];
	timeval curtime;
	gettimeofday(&curtime,NULL);

	res=(tevent.tv_sec-curtime.tv_sec)*1000000;
	res+=(tevent.tv_usec-curtime.tv_usec);
	res=labs(res);
	return (res>MAX_SLEEP_USEC) ? MAX_SLEEP_USEC : res;
		
}


bool Timer::IsExpired(unsigned int iID){
	timeval tevent=timMap[iID];
	timeval curtime;
	gettimeofday(&curtime,NULL);
	return !( curtime.tv_sec<tevent.tv_sec || (curtime.tv_sec==tevent.tv_sec && curtime.tv_usec<tevent.tv_usec ));
	
}

void Timer::AddNewTimer(unsigned int iID,unsigned short iTimeout){
	timeval curTime;
	gettimeofday(&curTime,NULL);
	curTime.tv_sec+=iTimeout;
	pthread_mutex_lock(&mapMutex);
	timMap.insert(timMap.end(),pair<unsigned int,timeval>(iID,curTime));
	pthread_mutex_unlock(&mapMutex);
}

void Timer::KillTimer(unsigned int iID){
	pthread_mutex_lock(&mapMutex);
	if(timMap.find(iID)!=timMap.end())
		timMap.erase(iID);
	pthread_mutex_unlock(&mapMutex);
	
}

