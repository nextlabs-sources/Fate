#include "stdafx.h"
#include "FileShareRegistry.h"

/***************************  Begin FileShareRegistryEntry function implementations **********/

FileShareRegistryEntry::FileShareRegistryEntry(const wstring& localPath, const wstring& fileSharePath) {
    this->localPath = new wstring(localPath);
    this->fileSharePath = new wstring(fileSharePath);
}

FileShareRegistryEntry::~FileShareRegistryEntry() {
	delete this->localPath;
	delete this->fileSharePath;

	// Deallocate duplicate shares by clearing set
	this->duplicateFileSharePaths.clear();
}

const wstring& FileShareRegistryEntry::getLocalPath() const {
    return *(this->localPath);
}

const wstring& FileShareRegistryEntry::getSharePath() const {
    return *(this->fileSharePath);
}

void FileShareRegistryEntry::addDuplicatePath(const wstring& duplicatePathToAdd) {
    // Check against original share path before insert
	if (duplicatePathToAdd.compare(this->getSharePath()) != 0) {
		this->duplicateFileSharePaths.insert(duplicatePathToAdd);
    }
}

hash_set<wstring>& FileShareRegistryEntry::getDuplicateSharePaths() {
    return this->duplicateFileSharePaths;
}

bool FileShareRegistryEntry::operator==(const FileShareRegistryEntry& other)
{
    return (*this->localPath == *other.localPath);
}

bool FileShareRegistryEntry::operator!=(const FileShareRegistryEntry& other)
{
    return (*this->localPath != *other.localPath);
}

/***************************  End FileShareRegistryEntry function implementations **********/
/***************************  Begin FileShareRegistry function implementations *****************/
FileShareRegistry::FileShareRegistry(const wstring& serverName) {
    this->serverName = new wstring(serverName);
}

FileShareRegistry::~FileShareRegistry() {
    delete this->serverName;

	hash_map<wstring, FileShareRegistryEntry*>::iterator fileShareRegistryEntryIterator = this->fileShareRegistry.begin();
	while (fileShareRegistryEntryIterator != this->fileShareRegistry.end()) {
		hash_map<wstring, FileShareRegistryEntry*>::iterator itemToRemove = fileShareRegistryEntryIterator;
		fileShareRegistryEntryIterator++;

		FileShareRegistryEntry* nextEntry = itemToRemove->second;		
		this->fileShareRegistry.erase(itemToRemove);
		delete nextEntry;
	}
}

const wstring& FileShareRegistry::getServerName() const {
    return *(this->serverName);
}

void FileShareRegistry::addShare(const wstring& localPath, const wstring& sharePath) {
    // Determine if this is a new entry or one which must be augemented
    FileShareRegistryEntry* registryEntryForLocalPath = NULL;
    hash_map<wstring,FileShareRegistryEntry*>::iterator foundEntry = this->fileShareRegistry.find(localPath);
    if (foundEntry == this->fileShareRegistry.end()) {
        registryEntryForLocalPath = new FileShareRegistryEntry(localPath, sharePath);
        this->fileShareRegistry[localPath] = registryEntryForLocalPath;
    } else {
        registryEntryForLocalPath = foundEntry->second;
        registryEntryForLocalPath->addDuplicatePath(sharePath);
    }

    findAndAddShareDuplicates(*registryEntryForLocalPath);
}

void FileShareRegistry::findAndAddShareDuplicates(const FileShareRegistryEntry& modifiedEntry) {
    const wstring& modifiedEntryLocalPath = modifiedEntry.getLocalPath();

    // Iterator through entries and add duplicate share where appropriate
    hash_map<wstring,FileShareRegistryEntry*>::iterator fileShareRegistryEntryIterator = this->fileShareRegistry.begin();
    while (fileShareRegistryEntryIterator != this->fileShareRegistry.end()) 
    {
        FileShareRegistryEntry* nextRegistryEntry = fileShareRegistryEntryIterator->second;
        if (*nextRegistryEntry != modifiedEntry) {

            const wstring& nextLocalPath = nextRegistryEntry->getLocalPath();

            wstring::size_type comparePosition = nextLocalPath.find(modifiedEntryLocalPath, 0);

            // This is the case when the local path provided is a substring of an already existing local path
            if (comparePosition != -1) {
                addDuplicates((FileShareRegistryEntry*)&modifiedEntry, nextRegistryEntry);
            }

            // This is the case when an already existing local path is a substring of the local path provided
            comparePosition = modifiedEntryLocalPath.find(nextLocalPath, 0);
            if (comparePosition != -1) {
                addDuplicates(nextRegistryEntry, (FileShareRegistryEntry*)&modifiedEntry);  
            }
        }
        fileShareRegistryEntryIterator++;
    }	

}

void FileShareRegistry::addDuplicates(FileShareRegistryEntry* sourceEntry, FileShareRegistryEntry* destinationEntry) {
    const wstring& sourceEntryLocalPath = sourceEntry->getLocalPath();
    const wstring& destinationEntryLocalPath = destinationEntry->getLocalPath();
    wstring* augmentedSharePath = buildAugmentedSharePath(sourceEntryLocalPath, destinationEntryLocalPath, sourceEntry->getSharePath());
    destinationEntry->addDuplicatePath(*augmentedSharePath);	
	
	// Not ideal to delete after passing to function.  Need to improve API to FileShareEntry
	delete augmentedSharePath;

    hash_set<wstring> duplicateShares = sourceEntry->getDuplicateSharePaths();
    hash_set<wstring>::iterator duplicateShareIterator = duplicateShares.begin();
    while (duplicateShareIterator != duplicateShares.end()) {
        const wstring* nextDuplicateShare = &*duplicateShareIterator;
        wstring* augmentedSharePath_l = buildAugmentedSharePath(sourceEntryLocalPath, destinationEntryLocalPath, *nextDuplicateShare);
        destinationEntry->addDuplicatePath(*augmentedSharePath_l);
		
		// Not ideal to delete after passing to function.  Need to improve API to FileShareEntry
		delete augmentedSharePath_l;

        duplicateShareIterator++;
    }  
}

wstring* FileShareRegistry::buildAugmentedSharePath(const wstring& shortPath, const wstring& longPath, const wstring& sharePathToAugment) {
    wstring* valueToReturn = new wstring(sharePathToAugment);
    wstring valueToAppend = longPath.substr(shortPath.length());
    valueToReturn->append(valueToAppend);
    return valueToReturn;
}

FileShareRegistry::Iterator FileShareRegistry::begin() {
    hash_map<wstring,FileShareRegistryEntry*>::iterator fileShareRegistryEntryIterator = this->fileShareRegistry.begin();
    return FileShareRegistry::Iterator(fileShareRegistryEntryIterator);
}

FileShareRegistry::Iterator FileShareRegistry::end() {
    hash_map<wstring,FileShareRegistryEntry*>::iterator fileShareRegistryEntryIterator = this->fileShareRegistry.end();
    return FileShareRegistry::Iterator(fileShareRegistryEntryIterator);
}
/***************************  End FileShareRegistry function implementations *****************/
/***************************  Being FileShareRegistry::iterator function implementations *****************/
FileShareRegistry::Iterator::Iterator(hash_map<wstring, FileShareRegistryEntry*>::iterator iteratorToWrap) {
    this->wrappedIterator = iteratorToWrap;
}

FileShareRegistry::Iterator& FileShareRegistry::Iterator::operator=(const Iterator& other)
{
    this->wrappedIterator = other.wrappedIterator;
    return(*this);
}

bool FileShareRegistry::Iterator::operator==(const Iterator& other)
{
    return(this->wrappedIterator == other.wrappedIterator);
}

bool FileShareRegistry::Iterator::operator!=(const Iterator& other)
{
    return(this->wrappedIterator != other.wrappedIterator);
}

FileShareRegistry::Iterator& FileShareRegistry::Iterator::operator++()
{
	(this->wrappedIterator)++;
	return(*this);
}

FileShareRegistryEntry& FileShareRegistry::Iterator::operator*()
{
    return(*(this->wrappedIterator->second));
}

FileShareRegistryEntry* FileShareRegistry::Iterator::operator->()
{
    return(this->wrappedIterator->second);
}
/***************************  End FileShareRegistry::iterator function implementations *****************/