#ifndef QUEUE_H
#define QUEUE_H

#include <exception>
#include <stdexcept>


template <typename T>
class Queue{
    private:
        struct node{
            T data;
            node* next;
            node(const T& data) : data(data), next(nullptr) {}
        };

        node* front;
        node* rear;
        int count;
    public:
        Queue() : front(nullptr), rear(nullptr), count(0) {}

        Queue(const Queue &obj) = delete;
        Queue& operator=(const Queue &other) = delete;

        ~Queue() {clear();}

        void clear() {
            while(!isEmpty()) {
                dequeue();
            }
        }

        bool isEmpty() const {
            if(count == 0 || front == nullptr) {return true;}
            return false; 
        }

        void enqueue(const T& data) {
            node* toAdd = new node(data);
            if(isEmpty()) {front = rear = toAdd;}
            else {
                rear->next = toAdd;
                rear = toAdd;
            }
            count++;
        }

        void dequeue() {
            if(isEmpty()) {throw std::underflow_error("Cannot delete from an empty queue\n");}
            
            node* temp = front;
            front = front->next;
            
            if(front == nullptr) {rear = nullptr;}
            
            delete temp;
            count--;
        }

        T& getFront() { 
            if(isEmpty()) {throw std::underflow_error("Queue is Empty\n;");}
            return front->data;
        }

        const T& getFront() const { 
            if(isEmpty()) {throw std::underflow_error("Queue is Empty\n;");}
            return front->data;
        }
        int size() const{return count;}


};



#endif