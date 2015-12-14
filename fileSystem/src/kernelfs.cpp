//File: kernelfs.cpp
#include "kernelfs.h"
#include "kernelfile.h"
#include "cache.h"

KernelFS* KernelFS::onlySample = nullptr;

KernelFS::KernelFS() : pt(ALPHASIZE), ft(ENTRYCNT){
}

KernelFS* KernelFS::sample(){
	if(onlySample == nullptr) 
		onlySample = new KernelFS();
	return onlySample;
}

char KernelFS::kmount(Partition *part){
	if(pt.fillRatio() == 1)
		return '0';//exception: full on part
	PartWrapper* pw = new PartWrapper(part);
	pt.insertKey(pw->getID(), pw);
	return pw->getName();
}

char KernelFS::kunmount(char part){
	PartWrapper* pw = pt.findKey(PartWrapper::toNumber(part) == 0);
	if(pw == 0)
		return 0;//excep: part with this part name doesn't exist
	pt.deleteKey(PartWrapper::toNumber(part));
	return 1;
}

char KernelFS::kformat(char part){
	PartWrapper*  pw = pt.findKey(PartWrapper::toNumber(part));
	if(pw == 0)
		return 0;//excep: part with this part name doesn't exist
	pw->clear();
	return 1;
}

EntryNum KernelFS::kexist(char* fname){
	PartWrapper* pw = pt.findKey(PartWrapper::parseName(fname));
	if(pw == 0)
		return 65;//excep: partition doesn't exist
	Directory *myDir = pw->rootDir();
	fname = FCB::parseName(fname);	
	for(EntryNum j = 0;j < ENTRYCNT; j++)
		if(strcmp(myDir[j]->name,fname) == 0)
			return j;//file found with given index
	return 64;//file not found
}

char KernelFS::kreadRootDir(char part, EntryNum entryNum,Directory &dir){
	PartWrapper* pw = pt.findKey(PartWrapper::toNumber(part));
	if(pw == 0)
		return 0;//excep: partition doesn't exist
	Directory *myDir = pw->rootDir();	

	char counter = 0;
	for(int i = entryNum; i < ENTRYCNT; i++)
		if(myDir[i]->name[0] != '\0')
			dir[counter++] = *myDir[i];
	if(counter < 64)
		return 0;
	return 1;
}

File* KernelFS::newFileOpened(PartWrapper* pw, char* fpath, char index, char mode){
	FCB* newFCB = new FCB(PartWrapper::parseName(fpath), index, mode);
	File* file = new File();
	file->myImpl->addFCB_ID(newFCB->getID());
	ft.insertKey(newFCB->getID(), newFCB);
	pw->fopen(newFCB->getID());
	return file;
}

File* KernelFS::startReading(char *fpath,char mode){
	EntryNum index = kexist(fpath);
	if(index > 63)
		return nullptr;//excep: file not found or no part with given name
	PartWrapper* pw = pt.findKey(PartWrapper::parseName(fpath));
	if(pw->getFormat())
		return nullptr;
	if(mode == 'r')
		pw->startReading(index);
	else
		pw->startWriting(index);
	return newFileOpened(pw, fpath, index, mode);
}

File* KernelFS::startWriting(char* fpath){
	EntryNum index = kexist(fpath);
	if(index == 65)
		return nullptr;//excep:: no partition with given name
	else if(index <= 63){
		kdelete(fpath);
		return nullptr;//excep: file exists and it's deleted
	}
		
	PartWrapper* pw = pt.findKey(PartWrapper::parseName(fpath));

	if(pw->getFormat())
		return nullptr;

	Directory *myDir = pw->rootDir();	

	
	for(EntryNum i = 0; i < ENTRYCNT; i++)
		if(myDir[i]->name[0] == '\0'){
			strcpy(myDir[index = i]->name, FCB::parseName(fpath));
			strcpy(myDir[index]->ext, FCB::parseExt(fpath));
			myDir[index]->indexCluster = pw->cluster();
			myDir[index]->size = ClusterSize;
			break;
		}

	if(index == 64)
		return nullptr; //excep: directory full

	pw->startWriting(index);
	return newFileOpened(pw, fpath, index, 'w');
}

File* KernelFS::kopen(char* fpath, char mode){
	switch(mode){
		case 'r':
			return startReading(fpath,mode);
			break;
		case 'w':
			return startWriting(fpath);
			break;
		case 'a':
			return startReading(fpath,mode);
			break;
		default:
			return nullptr;
	}
}
