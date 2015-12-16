//Fbufferile: fcb.h
#ifndef _fcb_h_
#define _fcb_h_
//#include "rw.h"
#include <cstring>

typedef unsigned long EntryNum;

class FCB;

class FCBid{
	friend class FCB;
	EntryNum entry;
	char part;
	char mode;
public:

	FCBid(){
		entry = 0;
		part = -1;
	}

	FCBid(EntryNum entry,char part,char mode){
		this->entry = entry;
		this->part = part;
		this->mode = mode;
	}

	char getMode(){
		return mode;
	}

	operator int() const{
		return (entry + part)*(entry + part + 1)/2 + part;
	}

	friend bool operator==(const FCBid& lhs, const FCBid& rhs);
	friend bool operator!=(const FCBid& lhs, const FCBid& rhs);
};

bool operator==(const FCBid& lhs, const FCBid& rhs){
	return lhs.part == rhs.part && lhs.entry == rhs.entry;
}
 
bool operator!=(const FCBid& lhs, const FCBid& rhs){
	return !(lhs == rhs);
}

class FCB{
	FCBid id;
	//ReadersWriters rw;
	unsigned long filesOp = 0;

public:
	FCB(FCBid id){
		this->id = id;
	}

	void startMode(char mode){
		/*if(mode == 'r')
			rw.startRead();
		else
			rw.startWrite();*/
		filesOp++;
	}

	void closeMode(char mode){
		/*if(mode == 'r')
			rw.stopRead();
		else
			rw.stopWrite();*/
		filesOp--;
	}

	unsigned long getFilesOpened(){
		return filesOp;
	}

	FCBid getID(){
		return id;
	}

	EntryNum getEntry(){
		return id.entry;
	}

	char getPart(){
		return id.part;
	}

	static void parseName(char *fpath, char *name){
		char p_len = 3;
		char *pch = strchr(fpath,'.');
		strncpy(name,fpath + p_len,pch - fpath - p_len);
	}
	static void parseExt(char *fpath,char *ext){
		char *pch = strchr(fpath,'.');
		strncpy(ext,fpath + (pch -fpath + 1),3);
	}
};
#endif
