#include <iostream>
#include <string>
#include <cassert>
#include "ds/Queue.h"
#include "ds/LinkedList.h"

int main() {
    //queue tests
    Queue<std::string> crawlQueue;
    assert(crawlQueue.isEmpty());

    crawlQueue.enqueue("index.html");
    crawlQueue.enqueue("linked_list.html");
    crawlQueue.enqueue("bst.html");

    assert(crawlQueue.size() == 3);
    assert(crawlQueue.getFront() == "index.html");

    crawlQueue.dequeue();
    assert(crawlQueue.getFront() == "linked_list.html");
    assert(crawlQueue.size() == 2);

    crawlQueue.clear();
    assert(crawlQueue.isEmpty());
    assert(crawlQueue.size() == 0);

    //LinkedList Tests
    LinkedList<int> hitList;
    assert(hitList.isEmpty());
    assert(hitList.find(10) == nullptr);

    hitList.pushBack(10);
    hitList.pushBack(20);
    hitList.pushFront(5); 

    assert(hitList.size() == 3);
    assert(hitList.getHead()->data == 5);
    assert(hitList.getHead()->next->data == 10);

    int* found = hitList.find(20);
    assert(found != nullptr && *found == 20);
    assert(hitList.find(99) == nullptr);

    hitList.deleteList();
    assert(hitList.isEmpty());
    assert(hitList.size() == 0);
    assert(hitList.getHead() == nullptr);

    std::cout << "[PASS] Queue<T> and LinkedList<T> passed all tests!\n";
    return 0;
}