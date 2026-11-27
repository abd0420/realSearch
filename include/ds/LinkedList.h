#ifndef LINKEDLIST_H
#define LINKEDLIST_H

template <typename T>
class LinkedList{
    
    public:
        struct node {
            T data;
            node* next;
            node(const T& data) : data(data), next(nullptr) {}
        };

    private:
        node* head;
        node* tail;
        int count;
    
    public:

        LinkedList() : head(nullptr), tail(nullptr), count(0) {}
        ~LinkedList() {deleteList();}

        LinkedList(const LinkedList &other) = delete;
        LinkedList& operator=(const LinkedList &other) = delete;


        bool isEmpty() const{
            if(!tail) {return true;}
            return false;
        }

        int size() const {return count;}

        void pushBack(const T& value) {
            node* toAdd = new node(value);
            if(isEmpty()) {head = tail = toAdd;}
            else{

                tail->next = toAdd;
                tail = toAdd;
            }
            count++;
        }
        
        void pushFront(const T& value) {
            node* toAdd = new node(value);
            if(isEmpty()) {head = tail = toAdd;}
            else{

                toAdd->next = head;
                head = toAdd;
            }
            count++;
        }

        void deleteList() {
            //deletion from the head to tail
            node* curr = head;
            while(curr) {
                node* temp = curr;
                curr = curr->next;
                delete temp;
            }
            head = tail = nullptr;
            count = 0;
        };
        node* getHead() {return head;}

        T* find(const T& target) {
            if(isEmpty()) {return nullptr;}
            node* curr = head;
            while(curr) {
                if(curr->data == target) {return &curr->data;}
                curr = curr->next;
            }
            return nullptr;
        }



};












#endif