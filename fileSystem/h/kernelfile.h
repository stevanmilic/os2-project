//File : kernelfile.h
#ifndef _kernelfile_h_
#define _kernelfile_h_
#include "fcb.h"
#include "kernelfs.h"
#include "index.h"

class KernelFile{
	friend class File;

	FCBid id;
	IndexAlloc* index;
	BytesCnt currByte = 0;//current byte position in file

	KernelFile() = default;
	
	char kwrite(BytesCnt,char*);
	BytesCnt kread(BytesCnt,char*);
	char seek(BytesCnt);
	BytesCnt filePos();
	char eof();
	BytesCnt getFileSize();
	char truncate();

public:
	void addFCBid(FCBid);
	static void kdeleteFile(PartWrapper*,EntryNum);
	~KernelFile();
};
#endif
