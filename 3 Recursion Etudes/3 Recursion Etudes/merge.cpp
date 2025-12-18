/* This function contains 4 functions validateQueue which takes a queue and ensures that it is in ascending order,
 * binaryMerge which takes two queues and iteratively creates a new queue that is the combination of the two queues in
 * ascending order, naiveMultiMerge which takes in a vector of queues and iterately calls binary merge to add them one by one,
 * recMultiMerge which recursively merges the vectors in the list
 */
#include <iostream>    // for cout, endl
#include "queue.h"
#include "SimpleTest.h"
using namespace std;


/* this helper function takes a queue and throws an error
 * if it is not sorted in ascending order
 */
void validateQueue(Queue<int> q) {
    while (q.size() > 1) { //if the queue has at least two elements
        int previous = q.dequeue();
        if (previous > q.peek()) { //check if the previous element is greater than the next (not sorted)
            error("Queue is not sorted");
        }
    }
    return; //if the queue has 0 or 1 items its sorted
}

/* this function takes two sorted queues a and b and merges them into
 * one single queue that is also sorted in ascending order
 */
Queue<int> binaryMerge(Queue<int> a, Queue<int> b) {
    Queue<int> result; //queue to store the sorted merged list

    //check that both queues are sorted
    validateQueue(a);
    validateQueue(b);

    while(!a.isEmpty()) { //condition that while a is not empty
        int aFirst = a.peek(); //check a because we know it is not empty
        if (!b.isEmpty()) { //condition that while b is not empty
            int bFirst = b.peek(); //check b because we know it is not empty
            //enqueue the smaller number
            if (aFirst < bFirst) {
                result.enqueue(a.dequeue());
            }
            else {
                result.enqueue(b.dequeue());
            }
        } else {
            break;
        }
    }

    //either a is empty in which case we fill up the merged list with rest of contents in B
    //or B is empty in which we fill up merged list with rest of contents in A
    if (a.isEmpty()) {
        while (!b.isEmpty()) {
            result.enqueue(b.dequeue());
        }
    } else {
        while (!a.isEmpty()) {
            result.enqueue(a.dequeue());
        }
    }

    return result;
}

/*
 * The function naiveMultiMerge assumes you have a correctly working
 * binaryMerge function, which this function calls to iteratively
 * merge a collection of sequences, starting off with an empty sequence
 * and repeatedly merging in an additional sequence until all are merged.
 * The code below is provided to you is implemented correctly and you
 * should not need to modify it.
 */
Queue<int> naiveMultiMerge(Vector<Queue<int>>& all) {
    Queue<int> result;

    for (Queue<int>& q : all) {
        result = binaryMerge(q, result);
    }
    return result;
}

/* this function takes a vector of queues and recursively splits
 * the vector into multiple parts that we run binaryMerge on
 * returning a merged sorted vector
 */
Queue<int> recMultiMerge(Vector<Queue<int>>& all) {
    Queue<int> result;
    int k = all.size();

    //base case
    if (k == 1) {
        return all[0];
    }

    Vector<Queue<int>> firstHalf = all.subList(0, k/2);
    Vector<Queue<int>> secondHalf = all.subList(k/2);
    return binaryMerge(recMultiMerge(firstHalf), recMultiMerge(secondHalf));
}


/* * * * * * Test Cases * * * * * */

Queue<int> createSequence(int size);
void distribute(Queue<int> input, Vector<Queue<int>>& all);

PROVIDED_TEST("binaryMerge, two short sequences") {
    Queue<int> a = {2, 4, 5};
    Queue<int> b = {1, 3, 3};
    Queue<int> expected = {1, 2, 3, 3, 4, 5};
    EXPECT_EQUAL(binaryMerge(a, b), expected);
    EXPECT_EQUAL(binaryMerge(b, a), expected);
}

PROVIDED_TEST("naiveMultiMerge, small collection of short sequences") {
    Vector<Queue<int>> all = {{3, 6, 9, 9, 100},
                             {1, 5, 9, 9, 12},
                             {5},
                             {},
                             {-5, -5},
                             {3402}
                            };
    Queue<int> expected = {-5, -5, 1, 3, 5, 5, 6, 9, 9, 9, 9, 12, 100, 3402};
    EXPECT_EQUAL(naiveMultiMerge(all), expected);
}

PROVIDED_TEST("recMultiMerge, compare to naiveMultiMerge") {
    int n = 20;
    Queue<int> input = createSequence(n);
    Vector<Queue<int>> all(n);
    distribute(input, all);
    EXPECT_EQUAL(recMultiMerge(all), naiveMultiMerge(all));
}

PROVIDED_TEST("Time binaryMerge operation") {
    int n = 1000000;
    Queue<int> a = createSequence(n);
    Queue<int> b = createSequence(n);
    TIME_OPERATION(a.size() + b.size(), binaryMerge(a, b));
}

PROVIDED_TEST("Time naiveMultiMerge operation") {
    int n = 11000;
    int k = n/10;
    Queue<int> input = createSequence(n);
    Vector<Queue<int>> all(k);
    distribute(input, all);
    TIME_OPERATION(input.size(), naiveMultiMerge(all));
}
PROVIDED_TEST("Time recMultiMerge operation") {
    int n = 90000;
    int k = n/10;
    Queue<int> input = createSequence(n);
    Vector<Queue<int>> all(k);
    distribute(input, all);
    TIME_OPERATION(input.size(), recMultiMerge(all));
}


/* Test helper to fill queue with sorted sequence */
Queue<int> createSequence(int size) {
    Queue<int> q;
    for (int i = 0; i < size; i++) {
        q.enqueue(i);
    }
    return q;
}

/* Test helper to distribute elements of sorted sequence across k sequences,
   k is size of Vector */
void distribute(Queue<int> input, Vector<Queue<int>>& all) {
    while (!input.isEmpty()) {
        all[randomInteger(0, all.size()-1)].enqueue(input.dequeue());
    }
}

//student test for binaryMerge
STUDENT_TEST("many test cases to prove that binaryMerge works") {
    Queue<int> aQ = {1, 2, 4, 5, 6, 7};
    Queue<int> bQ = {3, 5, 6, 7, 8, 12};
    Queue<int> answer = {1, 2, 3, 4, 5, 5, 6, 6, 7, 7, 8, 12};
    EXPECT_EQUAL(binaryMerge(aQ, bQ), answer);

    aQ = {-1, 6, 10, 20};
    bQ = {-3, -2, 90, 91};
    answer = {-3, -2, -1, 6, 10, 20, 90, 91};
    EXPECT_EQUAL(binaryMerge(aQ, bQ), answer);
}

//student test for binaryMerge for unsorted list
STUDENT_TEST("many test cases to prove that binaryMerge throws an error for unsorted lists") {
    Queue<int> aQ = {1, 2, 4, 5, 6, 7};
    Queue<int> bQ = {12, 3, 5, 6, 7, 8};
    EXPECT_ERROR(binaryMerge(aQ, bQ));

    aQ = {20, -1, -4, 10};
    bQ = {-3, -2, 90, -100};
    EXPECT_ERROR(binaryMerge(aQ, bQ));

    aQ = {-1, 6, 10, 20};
    bQ = {-3, -2, 90, 91};
    EXPECT_NO_ERROR(binaryMerge(aQ, bQ));
}

//helper function to run multiple binarySearches to scale up the time
void manybinaryMerges(Queue<int> a, Queue<int> b) {
    const int NUM_ITERATIONS = 700000;

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        binaryMerge(a, b);
    }
}

//student test to find the time operation of binaryMerge
STUDENT_TEST("Time operation on binaryMerge") {
    Queue<int> aQ = {1};
    Queue<int> bQ = {2};
    TIME_OPERATION(aQ.size() + bQ.size(), manybinaryMerges(aQ, bQ));

    aQ = {1, 2};
    bQ = {2, 3};
    TIME_OPERATION(aQ.size() + bQ.size(), manybinaryMerges(aQ, bQ));

    aQ = {1, 2, 4, 6};
    bQ = {2, 5, 6, 7};
    TIME_OPERATION(aQ.size() + bQ.size(), manybinaryMerges(aQ, bQ));

    aQ = {1, 5, 7, 9, 10, 12, 14, 15};
    bQ = {2, 3, 5, 7, 20, 42, 52, 60};
    TIME_OPERATION(aQ.size() + bQ.size(), manybinaryMerges(aQ, bQ));
}

//student test for naiveMultiMerge
STUDENT_TEST("2-3 test cases for naiveMultiMerge") {
    Vector<Queue<int>> vectorQueues = {{}, {}, {}, {}};
    Queue<int> answer = {};
    EXPECT_EQUAL(naiveMultiMerge(vectorQueues), answer);

    vectorQueues = {};
    answer = {};
    EXPECT_EQUAL(naiveMultiMerge(vectorQueues), answer);

    vectorQueues = {{1, 2}, {3, 4}, {}, {}};
    answer = {1, 2, 3, 4};
    EXPECT_EQUAL(naiveMultiMerge(vectorQueues), answer);
}

//helper function to run multiple naiveMultiMerge to scale up the time
void manyNaiveMultiMerge(Vector<Queue<int>> all, int iterations) {
    const int NUM_ITERATIONS = iterations;

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        naiveMultiMerge(all);
    }
}


// student test to find the time operation of naiveMultiMerge
STUDENT_TEST("Time trial naiveMultiMerge, double n, keep k constant") {
    Vector<Queue<int>> qVector;

    for (int qSize = 1; qSize <= 16; qSize *= 2) {
        for (int k = 0; k < 5; k++) {
            Queue<int> q = createSequence(qSize);
            qVector.add(q);
        }
        int n = qSize * 5;
        TIME_OPERATION(n, manyNaiveMultiMerge(qVector, 50000));
    }
}

STUDENT_TEST("Time trial naiveMultiMerge, keep n constant, increase k by 1") {
    int n = 60; //always the number of total elements
    int k = 1; //start at a small k value
    const int iterations = 5;  // do 5 doublings

    for (int i = 0; i < iterations && k <= n; i++) {
        int qSize = n / k; //may lose some accuracy due to rounding but necessary
        Vector<Queue<int>> qVector;

        for (int kVal = 0; kVal < k; kVal++) {
            Queue<int> q = createSequence(qSize);
            qVector.add(q);
        }

        TIME_OPERATION(k, manyNaiveMultiMerge(qVector, 50000));

        k += 1; //include one more queue
    }
}

// student test for recMultiMerge
STUDENT_TEST("testing recMultiMerge on different case and that it matches naiveMultiMerge") {
    Vector<Queue<int>> vectorQueues = {{1, 2}, {5, 6}, {1, 5, 6}, {3, 9, 12}};
    Queue<int> answer = {1, 1, 2, 3, 5, 5, 6, 6, 9, 12};
    EXPECT_EQUAL(recMultiMerge(vectorQueues), answer);

    EXPECT_EQUAL(recMultiMerge(vectorQueues), naiveMultiMerge(vectorQueues));

    vectorQueues = {{1, 2}, {3, 4}, {}, {}};
    answer = {1, 2, 3, 4};
    EXPECT_EQUAL(recMultiMerge(vectorQueues), answer);

    EXPECT_EQUAL(recMultiMerge(vectorQueues), naiveMultiMerge(vectorQueues));
}

//helper function to run multiple recMultiMerge to scale up the time
void manyRecMultiMerges(Vector<Queue<int>> all, int iterations) {
    const int NUM_ITERATIONS = iterations;

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        recMultiMerge(all);
    }
}

// student test to find the time operation of recMultiMerge
STUDENT_TEST("Time trial recMultiMerge, double n, keep k constant") {
    Vector<Queue<int>> qVector;

    for (int qSize = 1; qSize <= 16; qSize *= 2) {
        for (int k = 0; k < 5; k++) {
            Queue<int> q = createSequence(qSize);
            qVector.add(q);
        }
        int n = qSize * 5;
        TIME_OPERATION(n, manyRecMultiMerges(qVector, 50000));
    }
}

STUDENT_TEST("Time trial recMultiMerge, keep n constant, quadruple k") {
    int n = 200000; //always the number of total elements
    int k = 1; //start at a small k value
    const int iterations = 5;  // do 5 doublings

    for (int i = 0; i < iterations && k <= n; i++) {
        int qSize = n / k;
        Vector<Queue<int>> qVector;

        for (int kVal = 0; kVal < k; kVal++) {
            Queue<int> q = createSequence(qSize);
            qVector.add(q);
        }

        TIME_OPERATION(k, manyRecMultiMerges(qVector, 10));

        k *= 4; //quadruple the number of queues
    }
}

//recMultiMerge on a sequence of size 1 million
STUDENT_TEST("recMultiMerge on a sequence of size 1 million") {
    int total = 1000000;
    int numQueues = 1000000;
    int perQueue = total / numQueues;

    Vector<Queue<int>> qVector;
    for (int i = 0; i < numQueues; i++) {
        Queue<int> q;
        for (int j = 0; j < perQueue; j++) {
            q.enqueue(i * perQueue + j);
        }
        qVector.add(q);
    }

    TIME_OPERATION(total, recMultiMerge(qVector));
}
