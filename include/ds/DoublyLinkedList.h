#pragma once

template <typename T>
class DoublyLinkedList {
public:
    struct Node { T data; Node* prev; Node* next; Node(const T& data) : data(data), prev(nullptr), next(nullptr) {} };
private:
    Node* head; Node* tail; int count;
    void unlink(Node* n){   
        if(!n) return;
        if(n->prev) n->prev->next = n->next; else head = n->next;
        if(n->next) n->next->prev = n->prev; else tail = n->prev;
        n->prev = n->next = nullptr;
        count--; }
    void linkFront(Node* n){
        n->prev = nullptr;
        n->next = head;
        if(head) head->prev = n;
        head = n;
        if (!tail) tail = n;
        count++;}
public:
    DoublyLinkedList(){
        head = nullptr; tail = nullptr; count = 0;
    } 
    
    ~DoublyLinkedList(){
        clear();
    }
    DoublyLinkedList(const DoublyLinkedList&) = delete;
    DoublyLinkedList& operator=(const DoublyLinkedList&) = delete;
    Node* pushFront(const T& value){
        Node* n = new Node(value);
        linkFront(n);
        return n;
    }
        
    void  moveToFront(Node* n){
        if(n == head || !n) return;
        unlink(n);
        linkFront(n);
    }
    void  remove(Node* n){
        if(!n) return;
        unlink(n);
        delete n;
    }           
    bool popBack(){
    if(!tail) return false;
    remove(tail);
    return true;
} 
    Node* getHead() const{
        return head;
    }  Node* getTail() const{
        return tail;
    }
    int   size() const{
        return count;
    }     bool isEmpty() const{
        return count == 0;
    }
    void  clear(){
        while(popBack()) {}
    }
};