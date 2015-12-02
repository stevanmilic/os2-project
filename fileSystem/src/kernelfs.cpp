//File: kernelfs.cpp
#include "kernelfs.h"

KernelFS* KernelFS::onlySample = nullptr;

KernelFS::KernelFS(){
	openedFiles_list = new List<FCB>();
	partInter = new (PartitionInterface*)[alphabetSize];
	buffer = new char[ClusterSize];
}

KernelFS::~KernelFS(){
	delete openedFiles_list;
	delete [] buffer,partInter;
}

KernelFS* KernelFS::sample(){
	if(onlySample == nullptr) 
		onlySample = new KernelFS();
	return onlySample;
}

char KernelFS::kmount(Partition *part){
	partInter[partCounter].part = part;	
	if(partCounter < alphabetSize)
		return 'A' + partCounter++;
	return '0';
}

char KernelFS::kunmount(char partName){
	partIndex = partName - 'A';
	if(partInter[partIndex]){
		enterCriticalSection(partIndex);
		delete partInter[partIndex];
		partInter[partIndex] = nullptr;
		return 1;
	}
	return 0;
}

char KernelFS::kformat(char partName){
	partIndex = partName - 'A';
	if(partInter[partIndex]){
		enterCriticalSection(partIndex);
		Partition *part = partInter[partIndex].part;
		std::memset(buffer,1,ClusterSize);
		unsigned long bvClusters = part->getNumOfClusters()/ClusterSize + part->getNumOfClusters() % ClusterSize ? 1 : 0//if number of clusters exceeds clustersize
		part->dirIndex = bvClusters + 1;// + 1 for dir index
		memset(buffer,0,part->dirIndex);
		for(unsigned long i = 0; i < part->dirIndex; i++){
			part->writeCluster(i, buffer);
			std::memset(buffer,1,ClusterSize);
		}
		std::memset(buffer,0,ClusterSize);
		std::memcpy(buffer,myDir,sizeof myDir);
		part->writeCluster(dirIndex,buffer);// if write = 1
		return 1;
	}
	return 0;
}

char KernelFS::kexist(char* fname){
	std::memset(buffer,0,ClusterSize);
	for(int i = 0; i < partCounter; i++){
		partInter[i].part->readCluster(partInter[i].dirIndex,buffer);
		std::memcpy(myDir,buffer,sizeof myDir);
		for(int j = 0;j < ENTRYCNT && myDir[j].name != 0; j++)
			if(myDir[j].name == fname)
				return 1;
	}
	return 0;
}

char KernelFS::kreadRootDir(char partName, EntryNum entryNum,Directory &dir){
	partIndex = partName - 'A';
	Partition* part = partInter[partIndex].part;
	std::memset(buffer,0,ClusterSize);
	part->readCluster(partInter[partIndex].dirIndex,buffer);
	std::memcpy(dir,buffer,ClusterSize);
	char counter;
	for(int i = entryNum; i < ENTRYCNT; i++)
		if(myDir[i].name != 0)
			dir[counter++] = myDir[i];
	if(counter < 64)
		return 0;
	return 1;
}


File* KernelFS::kopen(char* fpath, char mode){
	switch(mode){
		case 'r':
			char partIndex = fpath[0] - 'A';
			char* fname = getFileName(fpath);
			Partition* part = partInter[partIndex].part;
			enterCriticalSection(part,fname);
			std::memset(buffer,0,ClusterSize);
			part->readCluster(partInter[partIndex].dirIndex, buffer);
			std::memcpy(myDir,buffer,sizeof myDir);
			char index = -1;
			for(char i = 0; i < ENTRYCNT; i++)
				if(myDir[i].name == fname)
					index = i;
			if(index < 0)
				return nullptr;
			File* file = new File();
			FCB* newFCB = new FCB(myDir[index],partIndex,index,0);
			openedFiles_list.add(newFCB,fcbCounter);
			file->myImpl->setID(fcbCounter++);
			break;
		case 'w':
			//start creating a file
			//TO DO: 1)findEntry,2)OpenAnewEntry,3)loadAtributesOfFile,4)checkRights
			//5)findFileSomeSpace!
			//1)findEntry
			char partIndex = fpath[0] - 'A';
			char* fname = getFileName(fpath);
			Partition* part = partInter[partIndex].part;
			enterCriticalSection(part,fname);
			std::memset(buffer,0,ClusterSize);
			part->readCluster(partInter[partIndex].dirIndex, buffer);
			std::memcpy(myDir,buffer,sizeof myDir);
			char index = -1;
			for(char i = 0; i < ENTRYCNT; i++)
				//2)findEntry
				if(myDir[i].name == 0){
					//3)loadAtributes
					myDir[index = i].name = getName(fname);
					myDir[index].ext = getExt(fname);
					myDir[index].indexCluster = getNewCluster(part);
					myDir[index].size = ClusterSize;
				}
				else if(myDir[i].name == fname){
					if(index > 0){
						myDir[index].name = 0;
					kdelete(fpath);
					return nullptr;
				}
			if(index < 0)
				return nullptr;
			std::memcpy(buffer,myDir,sizeof myDir);
			part->writeCluster(partInter[partIndex].dirIndex,buffer);
			FCB* newFCB = new FCB(myDir[index],partIndex,index,0);
			openedFiles_list.add(newFCB,fcbCounter++);
			File* file = new File();
			file->myImpl->position(myDir[index].indexCluster);
			break;
		case 'a':
			char partIndex = fpath[0] - 'A';
			char* fname = getFileName(fpath);
			Partition* part = partInter[partIndex].part;
			enterCriticalSection(part,fname);
			std::memset(buffer,0,ClusterSize);
			part->readCluster(partInter[partIndex].dirIndex, buffer);
			std::memcpy(myDir,buffer,sizeof myDir);
			char index = -1;
			for(char i = 0; i < ENTRYCNT; i++)
				if(myDir[i].name == fname)
					index = i;
			if(index < 0)
				return nullptr;
			openedFiles_list.add(newFCB,fcbCounter++);
			ClusterNo cluster = findLastWrittenCluster(myDir[index].indexCluster);
			ByteCnt cursor = findLastWrittenByte(cluster);
			FCB* newFCB = new FCB(myDir[index],partIndex,index,cursor);
			File* file = new File();
			file->myImpl->position(cluster,cursor);
			break;
		default:
			return nullptr;
	}
	return file;
}
