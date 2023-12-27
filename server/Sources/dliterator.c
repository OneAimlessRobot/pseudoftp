#include "../Includes/preprocessor.h"
#include "../Includes/nodes.h"
#include "../Includes/comparator.h"
#include "../Includes/doublelist.h"
#include "../Includes/dliterator.h"



void* nextIt(dliterator* it){


	void* result=it->nextNode->mem;
	it->nextNode=it->nextNode->next;
	return result;

}
dliterator* initIt(DListW* list){
	
	dliterator* result= malloc(sizeof(dliterator));
	result->list= list;
	result->nextNode=list->head;

	return result;


}
void* hasNextIt(dliterator*it){

	//printf("No: %p\n",it->nextNode);
	return it->nextNode;

}
void rewindIt(dliterator*it){


	 it->nextNode=it->list->head;

}

void* nextItComp(dliteratorcomp* it){


	void* result=it->nextNode->mem;
	it->nextNode=it->nextNode->next;
	return result;

}
dliteratorcomp* initItComp(DListWComp* list){
	
	dliteratorcomp* result= malloc(sizeof(dliteratorcomp));
	result->list= list;
	result->nextNode=list->head;

	return result;


}
void* hasNextItComp(dliteratorcomp*it){


	return it->nextNode;

}
void rewindItComp(dliteratorcomp*it){


	 it->nextNode=it->list->head;

}
