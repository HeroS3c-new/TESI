#include "globals.h"
#ifndef EXCEPTION_H_
#define EXCEPTION_H_

class Exception
{
	private:
	string msg;
public:
	Exception(string message);
	string getMessage();
	virtual ~Exception();
};

#endif /*EXCEPTION_H_*/
