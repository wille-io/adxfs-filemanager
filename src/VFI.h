#ifndef VFI_H	
#define VFI_H

#include "header.h"
#include "List.h"

/* this class represents a index, which can be of type directory, device or a file
 * one index is identified by its device it is located in, its file-name, -size, -type, -position on device
 * and if it contains children directories (if it is a directory) and a parent */
class VFI // virtual file index
{
	public:
		void reset();
		VFI();
		~VFI();

		ubyte1  deviceID; // device id on which device the file is stored (so the file descriptor knows how to handle the file)
		char* nodeName; // shown name of file / directory
		int   fileSize; // maximum size of 65536 bytes
		ubyte1  fileType; // is directory, file or device
		int   lba; // logical block addressing - position of file on corresponding device by dev_id (pointer to file information, not the file data position)
      VFI*  parent; // parent of his children (of this VFI)
		ubyte1 fileSystemType; // to tell the FileSystemManager how to handle the filesystem on the coherant device
		ubyte1 iNode; // temporary (?) variable for file systems to recognize a special node, instead of doing a search on the whole filesystem on nextNode again
		// lba is so 1970 ... i prefer the inode idea !

		List<VFI*>* getChildrenList(); // checks if there are children, if directory - to save memory not every VFI is fully initialized
		void setChildrenList(List<VFI*>* childrenList);
		void addChild(VFI* child);
		VFI* lookupChildren(char* nodename);

	private:
		List<VFI*>* childrenList; // list of VFI pointer (only create if there are children, to save virtual memory)
};

#endif
