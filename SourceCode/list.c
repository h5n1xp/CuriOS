//
//  List.c
//
//  Created by Matt Parsons on 25/10/2020.
//  Copyright © 2020 Matt Parsons. All rights reserved.
//

#include "List.h"
#include "string.h"
#include "SystemLog.h"

void InitList(list_t* list){
    list->head = (node_t*)&list->tail;
    list->tail = NULL;
    list->pred = (node_t*)&list->head;
    
    list->node.size = sizeof(sizeof(list_t));
    list->node.nodeType = NODE_LIST;
    list->count = 0;
    
    list->lock.isLocked = false;
    
}


void AddHead( list_t* list, node_t* node){
    
    Insert(list,node,list->head->prev);
    
}
void AddTail(list_t* list, node_t* node){

    Insert(list,node,list->pred);
    
}

void Enqueue(list_t* list,node_t* node){

    node_t* pred = list->pred;
    
    while(pred->prev != NULL){
        
        if(pred->priority >= node->priority){
            Insert(list, node, pred);
            return;
        }
        
        pred = pred->prev;
    }
    
    
    AddHead(list, node);
}

void EnqueueSize(list_t* list,node_t* node){

    node_t* pred = list->pred;
    
    while(pred->prev != NULL){
        
        if(pred->size >= node->size){
            Insert(list, node, pred);
            return;
        }
        
        pred = pred->prev;
    }
    
    
    AddHead(list, node);
}

node_t* FindName(list_t* list, char* name){
    
    node_t* node = list->head;
    
    do{
        
        if(strcmp(node->name,name) == 0){
            return node;
        }
        
        node = node->next;
    }while(node->next != NULL);
    
    return NULL;
}


node_t* ItemAtIndex(list_t* list, uint64_t index){
    
    int64_t size = (int64_t) list->count - 1;
    
    if((int64_t)index > size){
        return NULL;
    }
    
    node_t* node = list->head;
    
    uint64_t counter = 0;
    
    do{
        
        if(counter == index){
            return node;
        }
        
        counter += 1;
        node = node->next;
    }while(node->next != NULL);
    
    return NULL;
    
    
    
}

void Insert(list_t* list, node_t* node, node_t* pred){
    
    node->next = pred->next;
    node->prev = pred;
    
    node->next->prev = node;
    pred->next = node;
    
    list->count +=1;
    
}

void Remove(list_t* list,node_t* node){
    
    list->count -=1;
    
    node->prev->next = node->next;
    node->next->prev = node->prev;
    
}
node_t* RemHead(list_t* list){

    node_t* node = list->head;
    
    if(node->next==NULL){
        return NULL;
    }
    
    Remove(list, node);
    return node;
}
node_t* RemTail(list_t* list){
    
    node_t* node = list->pred;
    
    if(node->prev == NULL){
        return NULL;
    }
    
    Remove(list, node);
    return node;
}
