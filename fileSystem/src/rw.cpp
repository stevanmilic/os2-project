#include "rw.h"

ReadersWriters::ReadersWriters(){
	InitializeCriticalSection(&csWriter);
	InitializeCriticalSection(&csReader);
	readClear = CreateEvent(NULL,TRUE,TRUE,NULL);
}

ReadersWriters::~ReadersWriters(){
	DeleteCriticalSection(&csWriter);
	DeleteCriticalSection(&csReader);
}
void ReadersWriters::startWrite(){
	EnterCriticalSection(&csWriter);
}

void ReadersWriters::stopWrite(){
	LeaveCriticalSection(&csWriter);
}

void ReadersWriters::startRead(){
	EnterCriticalSection(&csReader);
	if (++readcount == 1)
		EnterCriticalSection(&csWriter);
	LeaveCriticalSection(&csReader);
}

void ReadersWriters::stopRead(){
	EnterCriticalSection(&csReader);
	if (--readcount == 0)
		LeaveCriticalSection(&csWriter);
	LeaveCriticalSection(&csReader);
}
