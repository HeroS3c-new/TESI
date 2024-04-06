#include "Exception.h"

Exception::Exception(string message)
{
	msg=message;
	DEBUG("Exception throwed: " << msg );
}

string Exception::getMessage(){
	return msg;
}
Exception::~Exception()
{
}
