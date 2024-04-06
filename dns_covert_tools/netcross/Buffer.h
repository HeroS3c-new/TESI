#include "globals.h"
#ifndef BUFFER_H_
#define BUFFER_H_

typedef  unsigned char byte;
typedef std::vector<byte> bytebuf;

static const char *ENCODING_TABLE = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

class Buffer
{
private:
	bytebuf iBuf;

	static char CharEncode(unsigned char) ;
	unsigned char CharDecode(char); 	
public:
	Buffer();
	Buffer(bytebuf);
	Buffer(string&);
	Buffer(const char *,unsigned long);
	Buffer(const byte *,unsigned long);
	virtual  ~Buffer();
	void allocate(unsigned int);
	void clear();
	void append(const byte *,unsigned long);
	void append(Buffer&);
	void append(string&);
	void insert(const byte *,unsigned long);
	void insert(Buffer&);
	void insert(string&);
	byte At(unsigned int iPos);
	byte * toArray();
	string toString();
	unsigned int size();
	string Encode();
	void Decode(string&);
	void DeleteFirstBytes(unsigned int);	
	
	};

#endif /*BUFFER_H_*/
