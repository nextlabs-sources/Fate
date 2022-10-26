#ifndef _FileShareRegistryInclude_
#define _FileShareRegistryInclude_

#include <hash_map>
#include <hash_set>
#include <string>

using namespace std;
using namespace stdext;

/*
FileShareRegistryEntry is a single entry node in the FileShareRegistry.  It represents a single local path on the file system along with all of the share paths which map to the local path
*/
class FileShareRegistryEntry {
    const wstring* localPath;
    const wstring* fileSharePath;
	hash_set<wstring> duplicateFileSharePaths;

    FileShareRegistryEntry(const wstring& localPath, const wstring& fileSharePath);
	~FileShareRegistryEntry();

    /*
    Add a duplicate path to this registry entry
    */
    void addDuplicatePath(const wstring& duplicatePathToAdd); 
public:
    /*
    Retrieve the local path for this registry entry
    */
    const wstring& getLocalPath() const;

    /*
    Retrieve the original share path of this entry.  This can be either the DFS path or the local share path, depending upon which was encounrtered first
    */
    const wstring& getSharePath() const;

    /*
    Retrieve the duplicate share paths for the entry
    */
    hash_set<wstring>& getDuplicateSharePaths();
    bool operator==(const FileShareRegistryEntry& other);
    bool operator!=(const FileShareRegistryEntry& other);
    friend class FileShareRegistry;
};

/*
FileShareRegistry contains all of the share information found for a particular server
*/
class FileShareRegistry {
    /*
    The name of the server for which this registry is associated
    */
    const wstring* serverName;

    /*
    Hash Map from local path to registry entry
    */
    hash_map<wstring, FileShareRegistryEntry*> fileShareRegistry;

    /*
    Utility method to find and add all duplicate share paths when adding a share
    */
    void findAndAddShareDuplicates(const FileShareRegistryEntry& modifiedEntry);

    /*
    Utility method to all duplicate share paths resulting from augmenting path information to the source entry in order to create duplicate shares in the destination entry.  (e.g. c:\Public - \\machine\Public to c:\Public\nested - \\machine\Public\nested)
    */
    void addDuplicates(FileShareRegistryEntry* sourceEntry, FileShareRegistryEntry* destinationEntry);

    /*
    Utility method used to build duplicate share paths (see use)
    */
    wstring* buildAugmentedSharePath(const wstring& shortPath, const wstring& longPath, const wstring& sharePathToAugment);

public:
    FileShareRegistry(const wstring& serverName);
    ~FileShareRegistry();

    /*
    Retrieve the server name associated with this registry
    */
    const wstring& getServerName() const;

    /*
    Add a file share to this registry.  If the local path already exists in the registry, it's a substring of an already existing path, or an already existing path is a substring of it, the appropriate duplicate share paths will be added
    */
    void addShare(const wstring& localPath, const wstring& sharePath);

    /*
        An iterator to be used for iterating through registry entries
    */
    class Iterator : public iterator<std::input_iterator_tag, FileShareRegistry*> {
        hash_map<wstring, FileShareRegistryEntry*>::iterator wrappedIterator;
        Iterator(hash_map<wstring, FileShareRegistryEntry*>::iterator iteratorToWrap); 
    public:
        Iterator& operator=(const Iterator& other);
        bool operator==(const Iterator& other);
        bool operator!=(const Iterator& other);
        Iterator& operator++();
        FileShareRegistryEntry& operator*();
        FileShareRegistryEntry* operator->();

        friend class FileShareRegistry;
    };

    /*
        Retrieve an input iterator to iterate through the registry entries
    */
    FileShareRegistry::Iterator begin();

    /*
        Retrieve an input iterator pointing to the last entry in the registry
    */
    FileShareRegistry::Iterator end();
};
#endif