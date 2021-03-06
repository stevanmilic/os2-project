#include "kernelfile.h"
#include "iterator.h"

KernelFile::~KernelFile() {
	FCB *fcb = KernelFS::ft.findKey(id);
	fcb->closeMode(id.getMode());
	if (!fcb->fileOpened()) {
		PartWrapper* pw = KernelFS::pt.findKey(fcb->getPart());
		pw->fclose(fcb->getEntry());
		KernelFS::ft.deleteKey(id);
	}
	delete index;
}

void KernelFile::addFCBid(FCBid id) {
	this->id = id;
	FCB *fcb = KernelFS::ft.findKey(id);
	PartWrapper* pw = KernelFS::pt.findKey(fcb->getPart());
	index = new IndexAlloc(pw, fcb->getEntry());
	if (id.getMode() == 'a')
		seek(getFileSize());
}

char KernelFile::kwrite(BytesCnt len, char *writeBuffer) {
	if (id.getMode() == 'r')
		return 0;

	if (getFileSize())
		index->loadIndex('w');

	index->load(len, writeBuffer);

	Iterator* iter = index->createIterator('w');
	for (iter->onFirst(); !iter->done(); iter->next());
	currByte += iter->curr();;

	if (currByte > getFileSize()) {
		FCB *fcb = KernelFS::ft.findKey(id);
		PartWrapper* pw = KernelFS::pt.findKey(fcb->getPart());
		pw->setFileSize(fcb->getEntry(), currByte);
	}
	delete iter;
	return 1;
}

BytesCnt KernelFile::kread(BytesCnt len, char *readBuffer) {
	if (getFileSize()) {
		index->loadIndex();
		if (currByte + len > getFileSize())
			len = getFileSize() - currByte;
	}

	index->load(len, readBuffer);

	Iterator* iter = index->createIterator('r');
	for (iter->onFirst(); !iter->done(); iter->next());
	currByte += iter->curr();
	delete iter;
	return iter->curr();
}

char KernelFile::seek(BytesCnt len) {
	if (getFileSize()) {
		index->loadIndex();
		currByte = 0;
		if (currByte + len > getFileSize())
			len = getFileSize() - currByte;
	}

	index->load(len);

	Iterator* iter = index->createIterator('s');
	for (iter->onFirst(); !iter->done(); iter->next());
	currByte += iter->curr();
	delete iter;
	return 1;
}

void KernelFile::kdeleteFile(PartWrapper *pw, EntryNum entry) {
	IndexAlloc *dindex = new IndexAlloc(pw, entry);
	BytesCnt fileSize = pw->getFileSize(entry);
	if (fileSize)
		dindex->load(fileSize);
	dindex->loadIndex();
	Iterator* iter = dindex->createIterator('d');
	for (iter->onFirst(); !iter->done(); iter->next());
	delete iter;
	delete dindex;
}

BytesCnt KernelFile::filePos() {
	return currByte;
}

char KernelFile::eof() {
	if (currByte == getFileSize())
		return 1;
	return 0;
}

BytesCnt KernelFile::getFileSize() {
	FCB *fcb = KernelFS::ft.findKey(id);
	PartWrapper* pw = KernelFS::pt.findKey(fcb->getPart());
	return pw->getFileSize(fcb->getEntry());
}

char KernelFile::truncate() {
	//TO DO: implement this :)
	return 1;
}
