#include "globals.h"
#ifndef CONFPARSER_H_
#define CONFPARSER_H_

#define MAX_ROW_SIZE 512
class ConfParser
{
private:
	map<string, map<string,string> > vars;

public:
	ConfParser();
	void ParseFile(string);
	string getVar(string,string);	
	virtual ~ConfParser();
};

#endif /*CONFPARSER_H_*/
