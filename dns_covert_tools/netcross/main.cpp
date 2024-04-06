/* ----------------------------------------------------------------------------
    Netcross - internet anyhow

     (C) 2006 by Primiano Tucci

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  -------------------------------------------------------------------------- */

#include "globals.h"
bool DebugMessages;
char AppMode;
unsigned short ClientLinkID; //very very temporary hack

Link *lnk;  //App-global Link Object
Client *SrvRef;
ConfParser cp;

void NullHandler(int signo){}
void PrintUsage();

/*------------------------------------------------------------
 * Main
 *------------------------------------------------------------
 */
int main(int argc, char *argv[]){
	try{
		vector<string> nparams;
		
		string confPath="/etc/netcross.conf";
		//Parse the command line
		for(int i=0;i<argc;i++){
			if(strlen(argv[i])<2) continue;
			if(argv[i][0]=='-'){
				switch(argv[i][1]){
					case 'd':{
						DebugMessages=true;
						break;}
					case 'D':{
						if(fork()!=0) return 0;
						break;}
					case 'h':{
						PrintUsage();
						return 0;
					}
				}}
			else
				confPath=argv[i];
		}
		//parse the config file
		cp.ParseFile(confPath);

		AppMode=(cp.getVar("global","mode")=="server") ? MODE_SERVER : MODE_CLIENT;
		bool UseLosslessLink=(cp.getVar("global","uselosslesslink")=="true"); 
		
		
		string sTmp;
	
		//Read link type
		sTmp=cp.getVar("global","link");
		if(sTmp=="ns"){
				lnk=new NSLink();
		}
		else{
			cout << "Unsupported link type " << sTmp <<". Exiting" << endl;
			exit(1);
		}
		
		if(UseLosslessLink)
			lnk=new LosslessLink(*lnk);

		
		//Read Link-specific vars
		nparams=lnk->GetNeededParams();
		
		for(int i=0;i<nparams.size();i++){
			
			sTmp=cp.getVar("link",nparams[i]);
			if(sTmp=="")
			{
				cout << "Insert parameter " << 	nparams[i] << " : " ;
				cin >> sTmp;
				cout << endl;	
			}

			lnk->Parameters[nparams[i]]=sTmp;
		}
	
			
		//Iinitialize the link
		lnk->Init();
		
		
		if(AppMode==MODE_CLIENT)
			SrvRef=new Client(0,*lnk);
	
	
		//Register signal handler
		signal(SIGINT,NullHandler);
		
		while(1){
			pause();
			if(AppMode==MODE_CLIENT)
				lnk->Terminate();//delete lnk;
				
			return 0;				
		}
		
	
	
 }catch(Exception *e){
 	cout << "Application error: " << e->getMessage() << endl; 
 	exit(1);
 	}

	 
}

void PrintUsage(){
	cout << "Usage: netcross [-h] [-D] [-d] [/path/to/netcross.conf]\n\n"
		 <<  "\t-D: run in Daemon mode\n"
		 <<  "\t-d: show debug messages\n"
		 <<  "\t-h: show this help\n"
		 <<  "\nVersion : " << VERSION << "\n" ;
}
