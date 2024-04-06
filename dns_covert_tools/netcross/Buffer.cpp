#include "Buffer.h"


Buffer::Buffer()
{
	
}

Buffer::Buffer(const byte *fromChar,unsigned long Rsize){
 iBuf.insert(iBuf.begin(),fromChar,fromChar+Rsize);
}
Buffer::Buffer(const char *fromChar,unsigned long Rsize){
 iBuf.insert(iBuf.begin(),(byte *)fromChar,((byte *)fromChar)+Rsize);
}
Buffer::Buffer(bytebuf inBuf){
	iBuf=inBuf;
}
Buffer::Buffer(string &inStr){
	iBuf.insert(iBuf.begin(),inStr.begin(),inStr.end());
}

Buffer::~Buffer()
{
	clear();
}
void Buffer::allocate(unsigned int iSize){
	iBuf.reserve(iSize);	
}
void Buffer::clear(){
	iBuf.clear();	
}
void Buffer::append(const byte *inByte,unsigned long Rsize){
	iBuf.insert(iBuf.end(),inByte,inByte+Rsize);
}
void Buffer::append(Buffer &inBuf){
	iBuf.insert(iBuf.end(),inBuf.iBuf.begin(),inBuf.iBuf.end());
}
void Buffer::append(string &inStr){
	iBuf.insert(iBuf.end(),inStr.begin(),inStr.end());
}
void Buffer::insert(const byte *inByte,unsigned long Rsize){
	iBuf.insert(iBuf.begin(),inByte,inByte+Rsize);
}
void Buffer::insert(Buffer &inBuf){
	iBuf.insert(iBuf.begin(),inBuf.iBuf.begin(),inBuf.iBuf.end());
}
void Buffer::insert(string &inStr){
	iBuf.insert(iBuf.begin(),inStr.begin(),inStr.end());
}

byte Buffer::At(unsigned int iPos){
	if(iPos>=size()) return 0;
	return iBuf.at(iPos);	
}

unsigned int Buffer::size(){
	return iBuf.size();	
}
byte * Buffer::toArray(){
	return &(iBuf.begin().operator *());
}
string Buffer::toString(){
	return string((char *)&(iBuf.begin().operator *()),iBuf.size());	
}

char Buffer::CharEncode(unsigned char u) {
	if(u>64)
	 throw new Exception("Encoding Failed");

	return ENCODING_TABLE[u];
}

unsigned char Buffer::CharDecode(char u) {
	//TODO: Checks
	char *pos=index(ENCODING_TABLE,u);
	return pos==NULL ? 255 : pos-ENCODING_TABLE;
}

void Buffer::DeleteFirstBytes(unsigned int nChars){
	if(size()<nChars) return;
	iBuf.erase(iBuf.begin(),iBuf.begin()+nChars);	
	memset(&(iBuf.end().operator *()),0,iBuf.capacity()-iBuf.size());
	
}
string Buffer::Encode(){
	string output;
	unsigned int size=iBuf.size();
	unsigned int i,x,tCount;
 	char Chunk[4];
 	
 	tCount=floor((float)size/3);
 	
 	x=0;
 
 	for(i=0;i<tCount;i++){
  		Chunk[1]=CharEncode(iBuf.at(x) >> 2);
 		Chunk[0]=iBuf.at(x) & 0x03;
 		x++;
 		Chunk[2]=CharEncode(iBuf.at(x) >> 2);
 		Chunk[0]=Chunk[0] | ((iBuf.at(x) & 0x03) << 2);
 		x++;
 		Chunk[3]=CharEncode(iBuf.at(x) >> 2);
 		Chunk[0]=Chunk[0] | ((iBuf.at(x) & 0x03) << 4);
 		x++;

 		Chunk[0]=CharEncode(Chunk[0]) ;
 		output.append(Chunk,4);
 	}
 	
    if((tCount=size % 3)>0){
    	for(i=0;i<tCount;i++){
    		byte cChar=iBuf.at(x);
	    	Chunk[0]='_';
	    	Chunk[1]=CharEncode(cChar >> 2);
	    	Chunk[2]=CharEncode(cChar & 0x03);
	    	x++;
	    	output.append(Chunk,3);
		}	    		
    }
    return output;
}

void Buffer::Decode(string &instr){
	unsigned int size=instr.size();	
	unsigned int i=0,x=0;
	byte Chunk[4];
	iBuf.clear();
	iBuf.erase(iBuf.begin(),iBuf.end());

	for(i=0;i+3<size;i+=4){
		Chunk[3]=CharDecode(instr.at(x++));
		if(Chunk[3]==255) {x--; break;}
		Chunk[0]=(CharDecode(instr.at(x++)) << 2) | (Chunk[3] & 0x03);
		Chunk[1]=(CharDecode(instr.at(x++)) << 2) | ((Chunk[3] & 0x0C) >> 2);
		Chunk[2]=(CharDecode(instr.at(x++)) << 2) | ((Chunk[3] & 0x30) >> 4);	 
		append(Chunk,3);
	} 

	for(;i+2<size;i+=3){
		if(instr.at(x)=='_'){
			Chunk[0]=(CharDecode(instr.at(++x)) << 2) | (CharDecode(instr.at(++x)));
			append(Chunk,1);	
			x++;
		}
	}
	
}

