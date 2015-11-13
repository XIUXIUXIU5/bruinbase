#include "BTreeNode.h"
using namespace std;

//helper:
PageId* BTLeafNode::siblingId()
{
	char *p = buffer;
	p += PageFile::PAGE_SIZE;
	p -= sizeof(PageId);
	return (PageId *)p;
}

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf)
{ 
	if(rt = pf.read(pid,buffer) < 0)	
		return rt;
	return 0; 
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf)
{ 
	RC rc;
	if(rc = pf.write(pid,buffer) < 0)
		return rc;
	return 0; 
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{ 
	int count = 0;
	leafIndex* p = buffer;
	while(p != 0 && count < MAX_LEAF_ENTRY_NUM){
		p++;
		count++;
	}
	return count; 
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid)
{ 
	if(getKeyCount() >= MAX_LEAF_ENTRY_NUM)
		return RC_NODE_FULL;
	int eid;
	locate(key,eid);
	leafIndex * p = buffer;
	p += eid+1;
	int size = sizeof(leafIndex) * (getKeyCount() - eid+1);
	char *temp = (char *)malloc(size);
	memcpy(temp,p,size);
	*p->rid = rid;
	*p->key = key;
	p++;
	memcpy(p,temp, size);
	free(temp);
	return 0;
}

/*
 * Insert the (key, rid) pair to the node
 * and split the node half and half with sibling.
 * The first key of the sibling node is returned in siblingKey.
 * @param key[IN] the key to insert.
 * @param rid[IN] the RecordId to insert.
 * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
 * @param siblingKey[OUT] the first key in the sibling node after split.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
                              BTLeafNode& sibling, int& siblingKey)
{ 
	RC rc;
	int eid;
	locate(key, eid);
	if(eid + 1 >= (MAX_LEAF_ENTRY_NUM + 1)/2)
	{
		leafIndex *p = buffer;
		p += (MAX_LEAF_ENTRY_NUM + 1)/2;
		int size = sizeof(leafIndex) * (MAX_LEAF_ENTRY_NUM/2);
		leafIndex *temp = p;
		for(int i = 0; i < MAX_LEAF_ENTRY_NUM/2; i++)
		{
			if(rc = sibling.insert(p->key,p->rid) < 0)	
				return rc;
			p++;
		}
		sibling.setNextNodePtr(getNextNodePtr());
		//can't self original next ptr since we don't know sibling's page id
		sibling.insert(key,rid);
		memset(temp,0,size);
		setNextNodePtr(-1);
		int rid;
		sibling.readEntry(0,siblingKey,rid);
	}
	else{
		leafIndex *p = buffer;
		p += MAX_LEAF_ENTRY_NUM/2;
		siblingKey = p->key;
		int size = sizeof(leafIndex) * (MAX_LEAF_ENTRY_NUM/2 +1);
		leafIndex *temp = p;
		for(int i = 0; i < MAX_LEAF_ENTRY_NUM/2 + 1; i++)
		{
			if(rc = sibling.insert(p->key,p->rid) < 0)
				return rc;
			p++;
		}
		sibling.setNextNodePtr(getNextNodePtr());
		memset(temp,0, size);
		setNextNodePtr(-1);
		insert(key,rid);
	}
	return 0; 
}

/**
 * If searchKey exists in the node, set eid to the index entry
 * with searchKey and return 0. If not, set eid to the index entry
 * immediately after the largest index key that is smaller than searchKey,
 * and return the error code RC_NO_SUCH_RECORD.
 * Remember that keys inside a B+tree node are always kept sorted.
 * @param searchKey[IN] the key to search for.
 * @param eid[OUT] the index entry number with searchKey or immediately
                   behind the largest key smaller than searchKey.
 * @return 0 if searchKey is found. Otherwise return an error code.
 */
RC BTLeafNode::locate(int searchKey, int& eid)
{ 
	eid = -1;
	leafIndex *p = buffer;
	while(p->key <= searchKey)
	{
		eid++;
		if(p->key == searchKey)
			return 0;
		p++;
	}
	return RC_NO_SUCH_RECORD; 
}

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid)
{ 
	leafIndex *p = buffer;
	if(eid < 0)
		return RC_INVALID_CURSOR;
	p += eid;
	rid = p->rid;
	key = p->key;
	return 0; 
}

/*
 * Return the pid of the next slibling node.
 * @return the PageId of the next sibling node 
 */
PageId BTLeafNode::getNextNodePtr()
{ 
	return *siblingId(); 
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid)
{
	PageId *next = siblingId();
	*next = pid; 
	return 0; 
}

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{ return 0; }
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{ return 0; }

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{ return 0; }


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid)
{ return 0; }

/*
 * Insert the (key, pid) pair to the node
 * and split the node half and half with sibling.
 * The middle key after the split is returned in midKey.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @param sibling[IN] the sibling node to split with. This node MUST be empty when this function is called.
 * @param midKey[OUT] the key in the middle after the split. This key should be inserted to the parent node.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey)
{ return 0; }

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{ return 0; }

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{ return 0; }
