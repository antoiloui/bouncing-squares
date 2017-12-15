#include "List.h"

/*
 *
 */
static Node* NodeNew(List* l, void* data, Node* next, Node* prev);

/*
	FUNCTION NodeDispose
	
	Frees the memory allocated for a Node. Also frees the data associated
	whether the argument $l$ is different from NULL.
	
	PARAMETERS :
		- $n$ : a pointer to the node to be freed
		- $l$ : a pointer to the list containing the data free function.
				NULL not to free the data associated
	
	RETURN :
		void
*/
static void  NodeDispose(Node* n, List* l);

struct List_t
{
    Node* head;
    Node* tail;
    size_t numElem;
    void (*freefn)(void*);
};

struct Node_t
{
    Node* prev;
    Node* next;
    void* data;
    List* parentList;
};

Node* NodeNew(List* l, void* data, Node* next, Node* prev)
{
	if(l == NULL)
		return NULL;
		
	Node* n = (Node*) malloc(sizeof(Node));
	
	if(n == NULL)
		return NULL;
		
	n->data = data;
	n->next = next;
	n->prev = prev;
	n->parentList = l;
	
	return n;
}

void NodeDispose(Node* n, List* l)
{
	if(n == NULL)
		return;
	
	if(l != NULL)
		l->freefn(n->data);
	
	free(n);
}

List* ListNew(void (*freefn)(void*))
{
    if(freefn == NULL)
        return NULL;

    List* l = (List*) malloc(sizeof(List));

    if(l == NULL)
        return NULL;

	l->head = NULL;
	l->tail = NULL;
	l->numElem = 0;
	l->freefn = freefn;
	
	return l;
}

void ListDispose(List* l, bool freeData)
{
	if(l == NULL)
		return;
	
	// remove all elements from the list
	while(!ListEmpty(l))
		ListRemoveNode(l, l->head, freeData); 
	
	free(l);
}

Node* ListGetHead(List* l)
{
	return l == NULL ? NULL : l->head;
}

Node* ListGetTail(List* l)
{
	return l == NULL ? NULL : l->tail;
}

int ListGetNumElem(List* l)
{
	return l == NULL ? -1 : (int) l->numElem;
}

void* ListRemoveNode(List* l, Node* n, bool freeData)
{
	if(l == NULL || n == NULL || !NodeIsInList(n, l) || ListEmpty(0))
		return NULL;
		
	    // One element in the list
    if(ListGetNumElem(l) == 1)
    {
        l->head = NULL;
        l->tail = NULL;
    }
    else
    {
        // n is the head of the list
        if(n == l->head)
        {
            n->next->prev = NULL;
            l->head = n->next;
        }
        // n is the tail of the list
        else if(n == l->tail)
        {
            n->prev->next = NULL;
            l->tail = n->prev;
        }
        else
        {
            n->next->prev = n->prev;
            n->prev->next = n->next;
        }
    }
	
	l->numElem--;
	
	// the data must be freed
	if(freeData)
	{
		NodeDispose(n, l);
		return NULL;
	}

	// the data is returned
	void* data = NodeGetData(n);
	NodeDispose(n, NULL);
	return data;
}

void* ListReplaceNode(List* l, Node* n, void* data, bool freeData)
{
	if(l == NULL || data == NULL || !NodeIsInList(n, l))
		return NULL;
		
	Node* newNode = NodeNew(l, data, n->next, n->prev);
	
	if(newNode == NULL)
		return NULL;
		
	if(ListGetNumElem(l) == 1)
	{
		l->head = newNode;
		l->tail = newNode;
	}
	else
	{
		if(n == l->tail)
		{
			n->prev->next = newNode;
			l->tail = newNode;
		}
		else if(n == l->head)
		{
			n->next->prev = newNode;
			l->head = newNode;
		}
		else
		{
			n->prev->next = newNode;
			n->next->prev = newNode;
		}
	}
	
	// data in node replaced must be freed
	if(freeData)
	{
		NodeDispose(n, l);
		return NULL;
	}
	
	// data in node replaced must be returned
	void* nData = NodeGetData(n);
	NodeDispose(n, NULL);
	return nData;
}

Node* ListInsertAtBegin(List* l, void* data)
{
	// list non-empty
	if(l != NULL && !ListEmpty(l))
		return ListInsertBefore(l, l->head, data);

	// list empty
	Node* newNode = NodeNew(l, data, NULL, NULL);
	l->head = newNode;
	l->tail = newNode;
	l->numElem++;
	
	return newNode;
}

Node* ListInsertAtEnd(List* l, void* data)
{
	// list non-empty
	if(l != NULL && !ListEmpty(l))
		return ListInsertAfter(l, l->tail, data);
	
	// list empty
	Node* newNode = NodeNew(l, data, NULL, NULL);
	l->head = newNode;
	l->tail = newNode;
	l->numElem++;
	
	return newNode;
}

Node* ListInsertAfter(List* l, Node* n, void* data)
{
	if(l == NULL || n == NULL || data == NULL || !NodeIsInList(n, l))
		return NULL;

	Node* newNode = NodeNew(l, data, n->next, n);
	
	if(newNode == NULL)
		return NULL;

	if(n == l->tail)
		l->tail = newNode;
	else
		n->next->prev = newNode;
	
	n->next = newNode;

	l->numElem++;
	
	return newNode;
}

Node* ListInsertBefore(List* l, Node* n, void* data)
{
	if(l == NULL || n == NULL || data == NULL || !NodeIsInList(n, l))
		return NULL;
	
	Node* newNode = NodeNew(l, data, n, n->prev);
	
	if(newNode == NULL)
		return NULL;
		
	if(n == l->head)
		l->head = newNode;
	else
		n->prev->next = newNode;
		
	n->prev = newNode;
	
	l->numElem++;
	
	return newNode;
}

bool ListEmpty(List* l)
{
	return l == NULL ? false : l->numElem == 0;
}

Node* NodeGetNext(Node* n)
{
	return n == NULL ? NULL : n->next;
}

Node* NodeGetPrevious(Node* n)
{
	return n == NULL ? NULL : n->prev;
}

void* NodeGetData(Node* n)
{
	return n == NULL ? NULL : n->data;
}

bool NodeIsInList(Node* n, List* l)
{
	return n != NULL && l != NULL && !ListEmpty(l) && n->parentList == l;
}
