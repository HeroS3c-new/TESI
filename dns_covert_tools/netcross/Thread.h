#include "globals.h"
#ifndef THREAD_H_
#define THREAD_H_

template<class T> class Thread;

template<class T> static void * ThreadStarterBridge(void *iThread){
 Thread<T> *thrd=static_cast<Thread<T>* >(iThread); 
 thrd->_call();	
 return NULL;
}

template<class T> 
class Thread {
  private:
  	 bool running;
     T *hInst;
     void (T::*hMethod)();
	 pthread_t PT;
   public:
   	 Thread(){
		running=false;
		}
	
   	 virtual ~Thread(){
   	 	Stop();
   	 };	
   	 void _call()
	   	 {
		  (hInst->*hMethod)();
		 }
   	 void Attach(T *iInst,void (T::*iMethod)())
		{ 
			 hInst=iInst;
			 hMethod=iMethod;	
		 }
   	 void Start()
	   	{
		if(!hInst || !hMethod)
			throw new Exception("Cannot start thread. No entrypoint attached");
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);	
		if(pthread_create(&PT,&attr,ThreadStarterBridge<T>,(void *)this))
			throw new Exception("Thread creation failed");
		running=true;
		}
   	 void Stop()
		{
			if(!running) return;
			pthread_cancel(PT);
			running=false;
		}
   	 
};



#endif /*THREAD_H_*/
