#ifndef _NODE_H_
#define _NODE_H_

#include <String.h>
#include "Strings.h"

enum NodeType{
    basicItem = 0,
    returnType = 1,
    setUint8Value = 2,
    setUint16Value = 3,
    selectSDLine = 4
};

class Node{
public:
    uint8_t index;
    const String * textPtr;
    Node * parentPtr;
    Node * nextPtr;
    Node * childPtr;
    NodeType nodeType;
    void * modifyVariablePtr;
    static uint8_t nodesCount;
    static Node * rootPtr;

    Node(const String * aText, NodeType aType);
    void insert(Node * aNewNode);
    void rename(const String * aTextPtr);
    void setModifyVariable(void * aVariablePtr);
    uint8_t getChildrenCount();
};

Node::Node(const String * aText, NodeType aType){
    this->index = nodesCount++;
    this->textPtr = aText;
    this->parentPtr = NULL;
    this->nextPtr = NULL;
    this->childPtr = NULL;
    if(rootPtr == NULL)
        rootPtr = this;
    nodeType = aType;
}
void Node::insert(Node * aNewNode){

    Node * iteratorPtr = this->childPtr;
    if(this->childPtr == NULL){    //first child of a parent node
        if(this == rootPtr){   //selected parent is rootNode
            this->childPtr = aNewNode;
        }
        else{   //selected parent is not rootNode
            Node * returnNode = new Node(&returnNodeTxt, returnType);
            this->childPtr = returnNode;    //set pointer of parent node
            returnNode->nextPtr = aNewNode;
            returnNode->parentPtr = this;
        }
        
    }
    else{
        while(iteratorPtr->nextPtr != NULL)
            iteratorPtr = iteratorPtr->nextPtr;
        iteratorPtr->nextPtr = aNewNode;     //set pointer of left siebling node
    }
    aNewNode->parentPtr = this;   //set pointer to parent node
    aNewNode->nextPtr = NULL;
    aNewNode->childPtr = NULL;
}
void Node::rename(const String * aTextPtr){
    this->textPtr = aTextPtr;
}
void Node::setModifyVariable(void * aVariablePtr){
    modifyVariablePtr = aVariablePtr;
}
uint8_t Node::getChildrenCount(){
    uint8_t count = 0;
    Node * child = this->childPtr;
    while(child != NULL){
        ++count;
        child = child->nextPtr;
    }
    return count;
}
uint8_t Node::nodesCount = 0;
Node * Node::rootPtr = NULL;

#endif /*_NODE_H_*/