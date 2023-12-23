#ifndef DLITERATOR_H
#define DLITERATOR_H


typedef struct dliterator{

DListW* list;
DList nextNode;

}dliterator;


void* nextIt(dliterator* it);
dliterator* initIt(DListW* list);
void* hasNextIt(dliterator*it);
void rewindIt(dliterator*it);


typedef struct dliteratorcomp{

DListWComp* list;
DList nextNode;

}dliteratorcomp;


void* nextItComp(dliteratorcomp* it);
dliteratorcomp* initItComp(DListWComp* list);
void* hasNextItComp(dliteratorcomp*it);
void rewindItComp(dliteratorcomp*it);

#endif
