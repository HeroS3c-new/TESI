#include "ConfParser.h"

ConfParser::ConfParser()
{
}

void ConfParser::ParseFile(string iFile){
	string row, lastsec,argl,argr;
	string::size_type epos;
	ifstream iStr;

	iStr.open(iFile.c_str(),ifstream::in);
	if(!iStr.is_open())
		throw new Exception("Cannot read conf file");
	

	
	while(iStr.good()){

		getline(iStr,row);

		if(row.size() < 2) continue;
		switch(row.at(0)){
			case '#':
			case ';':
			 	continue;
			 	 break;
			case '[':
				lastsec=row.substr(1,row.size()-2);
				break; 	
			default:
				if(lastsec=="") continue;
				epos=row.find('=',1);
				if(epos==string::npos) continue;

				argl=row.substr(0,epos);
				argr=row.substr(epos+1,row.size()-epos);
				vars[lastsec][argl]=argr;
				break;
		}
	}
	iStr.close();
}
string ConfParser::getVar(string iSection,string iKey){
	if(vars.find(iSection)==vars.end()) return "";
	if(vars[iSection].find(iKey)==vars[iSection].end()) return "";
	return 	vars[iSection][iKey];
}

ConfParser::~ConfParser()
{
}
