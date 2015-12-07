//File : kernelfile.h
#ifndef _kernelfile_h_
#define _kernelfile_h_
#include "mutex.h"
#include "windows.h"
#include "kernelfs.h"

#define signal(x) ReleaseSemaphore(x,1,NULL)
#define wait(x) WaitForSingleObject(x,INFINITE)

typedef unsigned long FCB_ID;

class KernelFile{
	private:
		friend class File;
		friend class KernelFS;

		HANDLE sem;
		HashTable<FCB_ID> openedFiles;

		KernelFile();
		
		char kwrite(BytesCnt,char*);
		BytesCnt kread(BytesCnt,char*);
		char seek(BytesCnt);
		BytesCnt filePos();
		char eof();
		BytesCnt getFileSize();
		char truncate();
		void setID(unsigned long);
	public:
		~KernelFile();
};
#endif
