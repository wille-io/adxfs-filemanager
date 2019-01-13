#ifndef LIST_H
#define LIST_H

#include "header.h"

#warning Problem: CPP + H in one file = overhead !

/* this class is a container for anonymous classes and represents a list
 * further, all methods have to be in the header file, because of limitations while using templates */
template <class T>
class List
{
	public:
      struct node
		{
         T     ptr;
         node* link;
      };

		List();
		~List();

      node* firstNode;
      node* currentPosition; // list position indicator for nextNode() & rewind() functions

      void appendEntry(T ptr);
      void deleteEntry(T ptr);

      void rewindNode(); // go to first node (if there is one)
      bool nextNode(); // step to next node, returns false if there is no next node

      node* getFirstNode();
      node* getNextNode();

      T getFirstEntry();
      T getNextEntry();

		ubyte1 count(); // get count of all items in the list
};

template <class T>
List<T>::List()
{
	firstNode = 0;
	currentPosition = 0;
}

template <class T>
List<T>::~List()
{
	this->rewindNode();
	while (currentPosition)
	{
		if (currentPosition->ptr)
		{
			T temp = currentPosition->ptr;
			delete temp;
			currentPosition->ptr = 0;
		}

		this->nextNode();
	}
}

template <class T>
void List<T>::rewindNode()
{
	currentPosition = firstNode; // no need for checks, firstNode is always 0 or a node*
}

template <class T>
bool List<T>::nextNode()
{
	if (currentPosition)
	{
		currentPosition = currentPosition->link;
		return true;
	}
	else
	{
		currentPosition = 0;
		return false;
	}
}

template <class T>
T List<T>::getFirstEntry() // advanced rewind() with safe return value
{
	this->rewindNode();

	if (!this->currentPosition) // there are no nodes in the list
		return 0;

	return this->currentPosition->ptr;
}

template <class T>
typename List<T>::node* List<T>::getFirstNode() // advanced rewind() with safe return value
{
	this->rewindNode();

	if (!this->currentPosition) // there are no nodes in the list
		return 0;

	return this->currentPosition;
}

template <class T>
T List<T>::getNextEntry() // advanced nextNode() with safe return value
{
	this->nextNode();
	if (/*this->nextNode() && */this->currentPosition) // only get next event if there is one ..
		return this->currentPosition->ptr;
	else
		return 0; // end of list
}



template <class T>
typename List<T>::node* List<T>::getNextNode() // advanced nextNode() with safe return value
{
	this->nextNode();
	if (/*this->nextNode() && */this->currentPosition) // only get next event if there is one ..
		return (node*)this->currentPosition;
	else
		return 0; // end of list
}

template <class T>
void List<T>::appendEntry(T ptr)
{
	node* temp = this->getFirstNode(); // pointer we are iterate with
	node* lastValidNode = 0;//temp; // the last valid pointer

	if (!temp) // seems to be the first entry
	{
		firstNode = new node;
		firstNode->ptr  = ptr;
		firstNode->link = 0;

		return;
	}

	while (temp)
	{
		lastValidNode = temp; // save working pointer
		temp = this->getNextNode(); // get next pointer

		if (!temp)
		{
			node* newNode = new node;
			newNode->ptr = ptr;
			newNode->link = 0;

			if (lastValidNode)
				lastValidNode->link = newNode;

			return;
		}
	}
}

template <class T>
void List<T>::deleteEntry(T ptr) // delete a node by its member "ptr"-pointer
{
	node* temp = this->getFirstNode();
	node* lastValidNode = 0;

	while (temp)
	{
		if (temp == this->firstNode && temp->ptr == ptr) // check if the current node is the first node AND IF THE POINTER FITS TO TEMP'S POINTER!!! mike u dumb b*tch!!
		{
			//std::cerr << "[firstNode] delted a entry! ptr = " << ptr << std::endl;
			firstNode = firstNode->link; // set firstNode's link-node to the new first node
			//memset(temp, 0, sizeof(node)); // safety issues...
			delete temp;
			this->currentPosition = firstNode;

			return;
		}

		lastValidNode = temp;
		temp = this->getNextNode(); // next pointer. Why here? <= it is not the first node & we need to set a last valid node..

		if (temp && temp->ptr == ptr)
		{
			//std::cerr << "[not firstNode] delted a entry! ptr = " << ptr << std::endl;
			lastValidNode->link = temp->link; // set the previus node to the next node after temp (which comes after lastValidNode) (worst case: temp->link = 0)
			//memset(temp, 0, sizeof(node));
			delete temp;
			this->currentPosition = lastValidNode; // go to last node, next iteration will go to the next node following after the old temp node
		}
	}
}

template <class T>
ubyte1 List<T>::count()
{
	ubyte1 result = 0;
	rewindNode();
	while (nextNode()) // +1 while next is possible
		result++;
	return result;
}

#endif
