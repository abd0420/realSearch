#pragma once
// Singly linked list. Used for HashTable collision chains (separate chaining).
// For the LRU cache we need the DOUBLY linked list in DoublyLinkedList.h.

template <typename T>
class LinkedList {
public:
    struct node {
        T data;
        node* next;
        node(const T& d) : data(d), next(nullptr) {}
    };

private:
    node* head;
    node* tail;
    int count;

public:
    LinkedList() : head(nullptr), tail(nullptr), count(0) {}
    ~LinkedList() { deleteList(); }

    // Copying would make two lists own the same nodes (double delete), so it
    // is disabled. Moving is allowed so a std::vector of lists can be built.
    LinkedList(const LinkedList&) = delete;
    LinkedList& operator=(const LinkedList&) = delete;

    LinkedList(LinkedList&& other) noexcept
        : head(other.head), tail(other.tail), count(other.count) {
        other.head = other.tail = nullptr;
        other.count = 0;
    }
    LinkedList& operator=(LinkedList&& other) noexcept {
        if (this != &other) {
            deleteList();
            head = other.head; tail = other.tail; count = other.count;
            other.head = other.tail = nullptr;
            other.count = 0;
        }
        return *this;
    }

    bool isEmpty() const { return count == 0; }
    int size() const { return count; }

    void pushBack(const T& value) {
        node* toAdd = new node(value);
        if (isEmpty()) { head = tail = toAdd; }
        else { tail->next = toAdd; tail = toAdd; }
        count++;
    }

    void pushFront(const T& value) {
        node* toAdd = new node(value);
        if (isEmpty()) { head = tail = toAdd; }
        else { toAdd->next = head; head = toAdd; }
        count++;
    }

    // Remove the first node. Returns false if the list is empty.
    bool popFront() {
        if (isEmpty()) return false;
        node* temp = head;
        head = head->next;
        if (head == nullptr) tail = nullptr;
        delete temp;
        count--;
        return true;
    }

    // Remove the first node for which pred(node.data) is true.
    // Returns true if something was removed. (Needed by HashTable::erase.)
    template <typename Pred>
    bool removeIf(Pred pred) {
        node* prev = nullptr;
        node* curr = head;
        while (curr) {
            if (pred(curr->data)) {
                if (prev) prev->next = curr->next; else head = curr->next;
                if (curr == tail) tail = prev;   // removed the last node
                delete curr;
                count--;
                return true;
            }
            prev = curr;
            curr = curr->next;
        }
        return false;
    }

    // Delete every node (head to tail).
    void deleteList() {
        node* curr = head;
        while (curr) {
            node* temp = curr;
            curr = curr->next;
            delete temp;
        }
        head = tail = nullptr;
        count = 0;
    }

    node* getHead() const { return head; }
    node* getTail() const { return tail; }

    // Needs operator== on T. Returns pointer to the stored data, or nullptr.
    T* find(const T& target) const {
        node* curr = head;
        while (curr) {
            if (curr->data == target) return &curr->data;
            curr = curr->next;
        }
        return nullptr;
    }
};


