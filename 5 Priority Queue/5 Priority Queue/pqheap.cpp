#include "pqheap.h"
#include "error.h"
#include "random.h"
#include "strlib.h"
#include "datapoint.h"
#include "SimpleTest.h"
using namespace std;

const int INITIAL_CAPACITY = 10;
const int NONE = -1; // used as sentinel index

/* The constructor initializes all of the member variables needed for
 * an instance of the PQHeap class. The allocated capacity
 * is initialized to a starting constant and a dynamic array of that
 * size is allocated. The count of filled slots is initially zero.
 */
PQHeap::PQHeap() {
    //initialize the max number of elements in the heap
    _numAllocated = INITIAL_CAPACITY;

    //initialize the heap
    _elements = new DataPoint[_numAllocated]();

    //set the number of elements in the heap to 0
    _numFilled = 0;
}

/* The destructor is responsible for cleaning up any resources
 * used by this instance of the PQHeap class. The array
 * memory that was allocated for the PQHeap is deleted here.
 */
PQHeap::~PQHeap() {
    delete [] _elements;
}

/* This function takes and element and adds it to the queue while maintaining
 * the shape of a min heap
 * If it exceeds the maximum capacity it should expand to fit more elements
 */
void PQHeap::enqueue(DataPoint elem) {
    //check if the array is full
    if (size() == _numAllocated) {
        enlargeArray();
    }

    //append the value to the end of the binary tree
    _elements[size()] = elem;

    //update the number of filled elements
    _numFilled++;

    //declare an index i that starts at the final index of elem
    int i = size() - 1;

    //loop through until i = 0
    while (getParentIndex(i) != NONE) {
        int parentIndex = getParentIndex(i);
        //if the elem is smaller than its parent swap them
        if (_elements[i].priority < _elements[parentIndex].priority) {
            swapElements(i, parentIndex);
        } else {
            break;
        }

        //update the index value to be the parent index
        i = parentIndex;
    }
}

/* This function returns the frontmost DataPoint in the min heap
 * representing the smallest priority value element without changing
 * the heap
 */
DataPoint PQHeap::peek() const {
    if (isEmpty()) {
        error("Cannot access front element of empty pqueue!");
    }
    return _elements[0];
}

/* This function returns removes and returns the frontmost element from the queue
 * by moving the bottom most element up and bubbling it back down
 * It also throws an error if you try to remove an empty queue
 */
DataPoint PQHeap::dequeue() {
    //throw an error if user tried to dequeue an empty queue
    if (isEmpty()) {
        error("Cannot dequeue an element from empty pqueue!");
    }

    //store the frontmost element
    DataPoint dp = _elements[0];

    //account for the edge case where the first element is your only element
    if (size() <= 1) {
        _numFilled--;
        return dp;
    }

    //move the last element to the top of the priority queue
    _elements[0] = _elements[size() - 1];

    //update my numFilled
    _numFilled--;

    //bubble down the last element
    int i = 0;

    while (getLeftChildIndex(i) != NONE) {
        //get the indexes of the two children
        int leftChild = getLeftChildIndex(i);
        int rightChild = getRightChildIndex(i);

        if (rightChild != NONE) {
            //store the datapoints of the two children
            DataPoint leftChildDP = _elements[leftChild];
            DataPoint rightChildDP = _elements[rightChild];

            //compare the two childs priorities and swap with the smaller child if our element is larger
            //ties picks the right child to swap with
            if (leftChildDP.priority < rightChildDP.priority) {
                if (_elements[i].priority > leftChildDP.priority) {
                    swapElements(i, leftChild);
                    i = leftChild;
                } else {
                    break;
                }
            } else {
                if (_elements[i].priority > rightChildDP.priority) {
                    swapElements(i, rightChild);
                    i = rightChild;
                } else {
                    break;
                }
            }
        } else {
            //store the datapoint of the left child
            DataPoint leftChildDP = _elements[leftChild];

            if (_elements[i].priority > leftChildDP.priority) {
                swapElements(i, leftChild);
                i = leftChild;
            } else {
                break;
            }
        }
    }

    return dp;
}

/* This function checks if the min heap is empty returning true
 * if it is and false if it has at least one element
 */
bool PQHeap::isEmpty() const {
    return size() == 0;
}

/* This member function returns the value in numFilled
 * represting the size of the priority queue
 */
int PQHeap::size() const {
    return _numFilled;
}

/* This function should empty the priority queue
 */
void PQHeap::clear() {
    _numFilled = 0;
}

/* Prints the contents of internal array for debugging purposes.
 */
void PQHeap::printDebugInfo(string msg) const {
    cout << msg << endl;
    for (int i = 0; i < size(); i++) {
        cout << "[" << i << "] = " << _elements[i] << endl;
    }
}

/* This function confirms the internal state of member variables appears valid.
 * In this case, check that the elements in the array are stored in priority order.
 * It also reports an error if a problem with the internal state is found.
 */
void PQHeap::validateInternalState() const {
    /*
     * If there are more elements than spots in the array, we have a problem.
     */
    if (_numFilled > _numAllocated) {
        error("Too many elements in not enough space!");
    }

    /* Loop over the array and compare priority of each element to each of its
     * child's priority.
     * If element a child at of any element has larger priority than its parent return the index of the
     * these two elements are out of order according to specification.
     * Report the problem using error.
     */
    for (int parent = 0; parent < size(); parent++) {
        //find the two childrens index
        int childOne = getLeftChildIndex(parent);
        int childTwo = getRightChildIndex(parent);

        //store the parents priority
        double parentPriority = _elements[parent].priority;

        //if the childOne index exists
        if (!(childOne == NONE)) {
            //print debug info if the child priority is less than the parents priority
            if (_elements[childOne].priority < parentPriority) {
                printDebugInfo("validateInternalState");
                error("PQArray has elements out of order at index " + integerToString(childOne));
            }
        }

        //if the childTwo index exists
        if (!(childTwo == NONE)) {
            //print debug info if the child priority is less than the parents priority
            if (_elements[childTwo].priority < parentPriority) {
                printDebugInfo("validateInternalState");
                error("PQArray has elements out of order at index " + integerToString(childTwo));
            }
        }
    }
}

/* This funciton calculates the index of the element that is the parent of the
 * specified child index.
 * If this child has no parent, return the sentinel value NONE.
 */
int PQHeap::getParentIndex(int child) const {
    //make sure the child is a valid index
    validateIndex(child);

    //calculate the parent index;
    int parent = (child - 1) / 2;

    //if the parent is at an invalid index return none
    //edge case child is 0 return none as well
    if (child == 0 || parent < 0) {
        return NONE;
    } else {
        //else return parent
        return parent;
    }
}

/* This function  calculates the index of the element that is the left child of the specified parent index.
 * If this parent has no left child, return the sentinel value NONE.
 */
int PQHeap::getLeftChildIndex(int parent) const {
    //make sure the parent is a valid index
    validateIndex(parent);

    //calculate the child index
    int child = (2 * parent) + 1;

    //if the child is at an invalid index return none
    if (child >= _numFilled) {
        return NONE;
    } else {
        //else return the child
        return child;
    }
}

/* This function calculates the index of the element that is the right child of the specified parent index.
 * If this parent has no right child, return the sentinel value NONE.
 */
int PQHeap::getRightChildIndex(int parent) const {
    //make sure the parent is a valid index
    validateIndex(parent);

    //calculate the child index
    int child = (2 * parent) + 2;

    //if the child is at an invalid index return none
    if (child >= _numFilled) {
        return NONE;
    } else {
        //else return the child
        return child;
    }
}

/*
 * This private member function is a helper that exchanges the element
 * at indexA with the element at indexB. In addition to being a handy
 * helper function for swapping elements, it also confirms that both
 * indexes are valid.  If you were to accidentally mishandle an index,
 * you will be so so glad this defensive protection is here to alert you!
 */
void PQHeap::swapElements(int indexA, int indexB) {
    validateIndex(indexA);
    validateIndex(indexB);
    DataPoint tmp = _elements[indexA];
    _elements[indexA] = _elements[indexB];
    _elements[indexB] = tmp;
}

/*
 * This private member function is a helper that confirms that index
 * is in within range of the filled portion of the element array,
 * raising an error if the index is invalid.
 */
void PQHeap::validateIndex(int index) const {
    if (index < 0 || index >= _numFilled) error("Invalid index " + integerToString(index));
}

void PQHeap::enlargeArray() {
    //double the capacity
    _numAllocated *= 2;
    
    //create a new pointer to an array with double the size
    DataPoint* _newElements = new DataPoint[_numAllocated]();

    //fill in the new array with the old elements
    for (int i = 0; i < size(); i++) {
        _newElements[i] = _elements[i];
    }

    //delete the old elements array
    delete[] _elements;

    //point the old elements array to the new elements array
    _elements = _newElements;
}

/* * * * * * Test Cases Below This Point * * * * * */

//testing the queues functionalities
STUDENT_TEST("testing the enqueue, peek, dequeue, and clear functions") {
    PQHeap pq;
    Vector<DataPoint> input = {
                               { "R", 44 }, { "A", 55 }, { "B", 34 }, { "K", 73 }, { "G", 22 },
                               { "V", 93 }, { "T", 14 }, { "O", 85 }, { "S", 64 } };

    pq.validateInternalState();

    for (DataPoint dp : input) {
        pq.enqueue(dp);
        pq.validateInternalState();
    }

    EXPECT_EQUAL(pq.peek(), { "T", 14 });

    while (!pq.isEmpty()) {
        pq.dequeue();
        pq.validateInternalState();
    }

    pq.clear();

    EXPECT_ERROR(pq.peek());
}

//specific test to check if enqueue works
STUDENT_TEST("PQHeap, enqueue only, validate at every step") {
    PQHeap pq;

    pq.enqueue({ "e", 2.718 });
    pq.validateInternalState();
    pq.enqueue({ "pi", 3.14 });
    pq.validateInternalState();
    pq.enqueue({ "phi", 1.618 });
    pq.validateInternalState();
    EXPECT_EQUAL(pq.size(), 3);
}

//specific test to check the size, isEmpty, and clear functions
STUDENT_TEST("PQHeap: operations size/isEmpty/clear") {
    PQHeap pq;

    EXPECT(pq.isEmpty());
    pq.clear();
    EXPECT_EQUAL(pq.isEmpty(), pq.size() == 0);
    pq.enqueue({ "", 7 });
    EXPECT_EQUAL(pq.size(), 1);
    pq.enqueue({ "", 5 });
    EXPECT_EQUAL(pq.size(), 2);
    pq.enqueue({ "", 5 });
    EXPECT_EQUAL(pq.size(), 3);
    pq.clear();
    pq.validateInternalState();
    EXPECT(pq.isEmpty());
    EXPECT_EQUAL(pq.size(), 0);
}


//specific edge cases for dequeue or peek on empty priority queue to see if it raises error
STUDENT_TEST("PQHeap: dequeue or peek on empty pqueue raises error") {
    PQHeap pq;
    DataPoint point = { "Programming Abstractions", 106 };

    EXPECT(pq.isEmpty());
    EXPECT_ERROR(pq.dequeue());
    EXPECT_ERROR(pq.peek());

    pq.enqueue(point);
    pq.dequeue();
    EXPECT_ERROR(pq.dequeue());
    EXPECT_ERROR(pq.peek());

    pq.enqueue(point);
    pq.clear();
    EXPECT_ERROR(pq.dequeue());
    EXPECT_ERROR(pq.peek());
}

//specific case to check if dequeue is valid for random inputs
STUDENT_TEST("PQHeap, dequeue, validate at every step") {
    PQHeap pq;

    pq.enqueue({ "e", 2.718 });
    pq.enqueue({ "pi", 3.14 });
    pq.enqueue({ "phi", 1.618 });

    for (int i = 0; i < 3; i++) {
        pq.dequeue();
        pq.validateInternalState();
    }
}

//specific test to check if the priority queue can enlarge array memory
STUDENT_TEST("PQHeap, test enlarge array memory") {
    for (int size = 5; size <= 500; size *= 5) {
        PQHeap pq;

        for (int i = 1; i <= size; i++) {
            pq.enqueue({"", double(i) });
        }
        pq.validateInternalState();

        for (int i = 1; i <= size; i++) {
            DataPoint expected = {"", double(i) };
            EXPECT_EQUAL(pq.dequeue(), expected);
        }
    }
}

//specific case to see if the priority queue can enqueue, dequeue a series of different values
//and still keep track of the different cases elements
STUDENT_TEST("PQHeap, sequence of mixed operations") {
    PQHeap pq;
    int size = 30;
    double val = 0;

    for (int i = 0; i < size; i++) {
        pq.enqueue({"", --val });
    }
    val = 0;
    for (int i = 0; i < pq.size(); i++) {
        DataPoint front = pq.peek();
        EXPECT_EQUAL(pq.dequeue(), front);
        pq.enqueue( { "", ++val });
    }
    EXPECT_EQUAL(pq.size(), size);
    val = 0;
    while (!pq.isEmpty()) {
        DataPoint expected = { "", ++val };
        EXPECT_EQUAL(pq.dequeue(), expected);
    }
}

//a test to see if the priority queue can handle many cycles of random elements
STUDENT_TEST("PQHeap stress test, cycle many random elements in and out") {
    PQHeap pq;
    int n = 0, maxEnqueues = 1000;
    double sumEnqueued = 0, sumDequeued = 0;

    setRandomSeed(42); // make test behavior deterministic

    DataPoint mostUrgent = { "", 0 };
    pq.enqueue(mostUrgent);
    while (true) {
        if (++n < maxEnqueues && randomChance(0.9)) {
            DataPoint elem = { "", randomInteger(-10, 10) + 0.5 };
            if (elem.priority < mostUrgent.priority) {
                mostUrgent = elem;
            }
            sumEnqueued += elem.priority;
            pq.enqueue(elem);
        } else {
            DataPoint elem = pq.dequeue();
            sumDequeued += elem.priority;
            EXPECT_EQUAL(elem, mostUrgent);
            if (pq.isEmpty()) break;
            mostUrgent = pq.peek();
        }
    }
    EXPECT_EQUAL(sumEnqueued, sumDequeued);
}

/* * * * * Provided Tests Below This Point * * * * */

PROVIDED_TEST("PQHeap example from writeup, validate each step") {
    PQHeap pq;
    Vector<DataPoint> input = {
        { "R", 4 }, { "A", 5 }, { "B", 3 }, { "K", 7 }, { "G", 2 },
        { "V", 9 }, { "T", 1 }, { "O", 8 }, { "S", 6 } };

    pq.validateInternalState();
    for (DataPoint dp : input) {
        pq.enqueue(dp);
        pq.validateInternalState();
    }
    while (!pq.isEmpty()) {
        pq.dequeue();
        pq.validateInternalState();
    }
}
