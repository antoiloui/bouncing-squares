#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

#include <stdlib.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct List_t List;
typedef struct Node_t Node;

/*
 *	FUNCTION ListNew :
 *		Initialize a new list that can contain any kind of data (pointers).
 *		The function $freefn$ is a function to free the kind  of data stored.
 *
 *	PARAMETERS :
 *		- $freefn$ : pointer to a function to free the kind of data stored
 *
 *	RETURN :
 *		- a pointer to the list
 *		- NULL on error
 */
List* ListNew(void (*freefn)(void*));

/*
 *	FUNCTION ListDispose :
 *		Destroy a list initialized with ListNew. $freeData$ specifies whether
 *      the data stored must be freed or not
 *
 *	PARAMETERS :
 *		- $l$ : a pointer to the list to free
 *      - $freeData$ : true whether the data stored must be freed, false otherwise
 *
 *	RETURN :
 *		 /
 */
void ListDispose(List* l, bool freeData);

/*
 *	FUNCTION ListGetHead :
 *		Returns a pointer of to the head node of the list $l$. 
 *
 *	PARAMETERS :
 *		- $l$ : a pointer to the list
 *
 *	RETURN :
 *		- a pointer of type 'Node*' to the head
 *		- NULL on error 
 */
Node* ListGetHead(List* l);

/*
 *	FUNCTION ListGetTail :
 *		Returns a pointer to the tail node of the list $l$.
 *
 *	PARAMETERS :
 *		- $l$ : a pointer to the list
 *
 *	RETURN :
 *		- a pointer of type 'Node*' to the tail
 *		- NULL on error
 */		
Node* ListGetTail(List* l);

/*
 *	FUNCTION ListGetNumElem :
 *		Returns the number of element in the list
 *
 *	PARAMETERS :
 *		- $l$ : a pointer to the list
 *
 *	RETURN :
 *		- '-1' on error
 *		- the number of element otherwise 
 */
int ListGetNumElem(List* l);

/*
 *	FUNCTION ListRemoveNode :
 *		Remove a node $n$ from the list $l$. $freeData$ specifies whether the 
 *		data stored must be freed
 *
 *	PARAMETERS :
 *		- $l$ : a pointer to the list
 *		- $n$ : a pointer to the node
 *		- $freeData$ : true to free data, false othertwise
 *
 *	RETURN :
 *		- NULL on error or when the data is freed and no error occured
 *		- a void pointer to the data stored (if 'freeData' is true)
 */
void* ListRemoveNode(List* l, Node* n, bool freeData);

/*
 *	FUNCTION ListReplaceNode :
 *		Replace a node $n$ from the list $l$ by a new node that will contain the data
 *		$data. $freeData$ specifies whether the data stored must be freed.
 *
 *	PARAMETERS :
 *		- $l$ : a pointer to the list
 *		- $n$ : a pointer to the node
 *		- $data$ : a pointer to the data to store in the new node
 *		- $freeData$ : true to free data, false othertwise
 *
 *	RETURN :
 *		- NULL on error or when the data (stored in the node replaced) is freed and no error occured
 *		- a void pointer to the data stored (if 'freeData' is true) in the node replaced
 */
void* ListReplaceNode(List* l, Node* n, void* data, bool freeData);

/*
 *	FUNCTION ListInsertAtBegin :
 *		Insert a new node at the beginning of the list. The data $data$ is stored in 
 *		in the new node.
 *
 *	PARAMETERS :
 *		- $l$ : a pointer to the list
 *		- $data$ : a pointer to the data to store in the new node
 *
 *	RETURN :
 *		- a pointer to the new node
 *		- NULL on error
 */
Node* ListInsertAtBegin(List* l, void* data);

/*
 *	FUNCTION ListInsertAtEnd :
 *		Insert a new node at the end of the list. The data $data$ is stored in 
 *		in the new node.
 *
 *	PARAMETERS :
 *		- $l$ : a pointer to the list
 *		- $data$ : a pointer to the data to store in the new node
 *
 *	RETURN :
 *		- a pointer to the new node
 *		- NULL on error
 */
Node* ListInsertAtEnd(List* l, void* data);

/*
 *	FUNCTION ListInsertAfter :
 *		Insert a new node after the node $n$. The data $data$ is stored in 
 *		in the new node.
 *
 *	PARAMETERS :
 *		- $l$ : a pointer to the list
 *		- $n$ : a pointer to the node 
 *		- $data$ : a pointer to the data to store in the new node
 *
 *	RETURN :
 *		- a pointer to the new node
 *		- NULL on error
 */
Node* ListInsertAfter(List* l, Node* n, void* data);

/*
 *	FUNCTION ListInsertBefore :
 *		Insert a new node before the node $n$. The data $data$ is stored in 
 *		in the new node.
 *
 *	PARAMETERS :
 *		- $l$ : a pointer to the list
 *		- $n$ : a pointer to the node 
 *		- $data$ : a pointer to the data to store in the new node
 *
 *	RETURN :
 *		- a pointer to the new node
 *		- NULL on error
 */
Node* ListInsertBefore(List* l, Node* n, void* data);

/*
 *	FUNCTION ListEmpty :
 *		...
 *
 *	PARAMETERS : 
 *		- $l$ : a pointer to the list
 *
 *	RETURN :
 *		- true if the list is empty
 *		- false otherwise
 */
bool ListEmpty(List* l);

/*
 *	FUNCTION NodeGetNext :
 *		Returns a pointer to the node following the node $n$ in its parent list.
 *
 *	PARAMETERS : 
 *		- $n$ : a pointer to the node
 *
 *	RETURN :
 *		- a pointer to the next node
 *		- NULL on error
 */
Node* NodeGetNext(Node* n);

/*
 *	FUNCTION NodeGetPrevious :
 *		Returns a pointer to the node preceeding the node $n$ in its parent list.
 *
 *	PARAMETERS : 
 *		- $n$ : a pointer to the node
 *
 *	RETURN :
 *		- a pointer to the previous node
 *		- NULL on error
 */
Node* NodeGetPrevious(Node* n);

/*
 *	FUNCTION NodeGetData :
 *		Returns a pointer to the data stored in the node $n$.
 *
 *	PARAMETERS : 
 *		- $n$ : a pointer to the node
 *
 *	RETURN :
 *		- a pointer to the next node
 *		- NULL on error
 */
void* NodeGetData(Node* n);

/*
 *	FUNCTION NodeIsInList :
 *		Returns true if the parent list of the node $n$ is $l$. 
 *
 *	PARAMETERS : 
 *		- $n$ : a pointer to the node
 *		- $l$ : a pointer to the list
 *
 *	RETURN :
 *		- true if $n$ belongs to $l$
 *		- false otherwise
 */
bool NodeIsInList(Node* n, List* l);

#endif // LIST_H_INCLUDED
