#include "pqclient.h"
#include "pqarray.h"
#include "pqheap.h"
#include "vector.h"
#include "strlib.h"
#include <sstream>
#include "SimpleTest.h"
using namespace std;

/* Given a reference to a vector of DataPoints v sort all the DataPoints by priority value
 * in increasing priority value.
 */
void pqSort(Vector<DataPoint>& v) {
    PQHeap pq;

    /* Using the Priority Queue data structure as a tool to sort, neat! */

    /* Add all the elements to the priority queue. */
    for (int i = 0; i < v.size(); i++) {
        pq.enqueue(v[i]);
    }

    /* Extract all the elements from the priority queue. Due
     * to the priority queue property, we know that we will get
     * these elements in sorted order, in order of increasing priority
     * value. Store elements back into vector, now in sorted order.
     */
    for (int i = 0; !pq.isEmpty(); i++) {
        v[i] = pq.dequeue();
    }
}

/* Given a stream of Datapoints stream and a k value which indicates how many of the top
 * priority numbers we want to store return a vector of the top k datapoints with the priority
 * values in descending order. The limitations are using no more O(k) space and also if there are
 * less than k datapoints in the stream return all of them in the order listed above.
 */
Vector<DataPoint> topK(istream& stream, int k) {
    //declare and initialize a PQ Array to store the highest k weights in descending order
    PQHeap topKSorted;

    //declare d datapoint cur to store our current data point from the stream
    DataPoint cur;

    //loop through the datapoints in stream
    while (stream >> cur) {

        //enqueue the cur DataPoint value
        topKSorted.enqueue(cur);

        //if we more than k values dequeue a value (the lowest priority value)
        if (topKSorted.size() > k) {
            topKSorted.dequeue();
        }
    }

    //declare and initialize a vector of datapoints to store the highest k weights in descending order
    Vector<DataPoint> topKElem(topKSorted.size());

    //loop through the priority queue, append to the end of the vector
    for (int i = topKSorted.size() - 1; i >= 0; i--) {
        DataPoint top = topKSorted.dequeue();
        topKElem[i] = top;
    }

    return topKElem;
}


/* * * * * * Test Cases Below This Point * * * * * */

/* Helper function that, given a list of data points, produces a stream from them. */
stringstream asStream(const Vector<DataPoint>& dataPoints) {
    stringstream result;
    for (const DataPoint& pt: dataPoints) {
        result << pt;
    }
    return result;
}

/* Helper function that, given range start and stop, produces a stream of values. */
stringstream asStream(int start, int stop) {
    stringstream result;
    for (int i = start; i <= stop; i++) {
        DataPoint pt = { "", double(i) };
        result << pt;
    }
    return result;
}

/* Helper function to fill vector with n random DataPoints. */
void fillVector(Vector<DataPoint>& vec, int n) {
    vec.clear();
    for (int i = 0; i < n; i++) {
        DataPoint pt = { "", randomReal(0, 100) };
        vec.add(pt);
    }
}

// testing the pqSort function with PQArray
STUDENT_TEST("time trail for the pqSort function") {
    for (int n = 10000; n <= 80000; n *= 2) {
        Vector<DataPoint> v;
        fillVector(v, n);
        TIME_OPERATION(n, pqSort(v));
    }
}


//testing the topK function with PQArray
PROVIDED_TEST("time trails for topK changing n") {
    int startSize = 4000000;
    int k = 10;
    for (int n = startSize; n <= 8*startSize; n *= 2) {
        Vector<DataPoint> input;
        fillVector(input, n);
        stringstream stream = asStream(input);
        TIME_OPERATION(n, topK(stream, k));
    }
}

PROVIDED_TEST("time trails for topK changing k") {
    int startSize = 5000;
    int n = 1000000;
    for (int k = startSize; k <= 8*startSize; k *= 2) {
        Vector<DataPoint> input;
        fillVector(input, n);
        stringstream stream = asStream(input);
        TIME_OPERATION(k, topK(stream, k));
    }
}


// testing the pqSort function with PQHeap
STUDENT_TEST("time trail for the pqSort function") {
    for (int n = 1000000; n <= 8000000; n *= 2) {
        Vector<DataPoint> v;
        fillVector(v, n);
        TIME_OPERATION(n, pqSort(v));
    }
}


//testing the topK function with PQHeap
PROVIDED_TEST("time trails for topK changing k") {
    int startSize = 50000;
    int n = 10000000;
    for (int k = startSize; k <= 8*startSize; k *= 2) {
        Vector<DataPoint> input;
        fillVector(input, n);
        stringstream stream = asStream(input);
        TIME_OPERATION(k, topK(stream, k));
    }
}


/* * * * * Provided Tests Below This Point * * * * */

PROVIDED_TEST("pqSort: vector of random elements") {
    setRandomSeed(137); // why might it be a good idea to set seed here?

    Vector<DataPoint> input;
    Vector<double> expected;
    fillVector(input, 100);
    for (DataPoint dp : input) {
        expected.add(dp.priority);
    }
    expected.sort();

    pqSort(input);
    for (int i = 0; i < input.size(); i++) {
        EXPECT_EQUAL(input[i].priority, expected[i]);
    }
}

PROVIDED_TEST("pqSort: time trial") {
    int startSize = 1000;
    for (int n = startSize; n < 10*startSize; n *= 2) {
        Vector<DataPoint> v;
        fillVector(v, n);
        TIME_OPERATION(n, pqSort(v));
    }
}


/* Constant used for sizing the tests below this point. */
const int kMany = 100000;

PROVIDED_TEST("topK: stream 0 elements, ask for top 1") {
    stringstream emptyStream = asStream({});
    Vector<DataPoint> expected = {};
    EXPECT_EQUAL(topK(emptyStream, 1), expected);
}

PROVIDED_TEST("topK: stream 1 element, ask for top 1") {
    stringstream stream = asStream({ { "" , 1 } });
    Vector<DataPoint> expected = { { "" , 1 } };
    EXPECT_EQUAL(topK(stream, 1), expected);
}

PROVIDED_TEST("topK: small hand-constructed input") {
    Vector<DataPoint> input = { { "A", 1 }, { "B", 2 }, { "C", 3 }, { "D", 4 } };

    stringstream stream = asStream(input);
    Vector<DataPoint> expected = { { "D", 4 }, { "C", 3 } };
    EXPECT_EQUAL(topK(stream, 2), expected);

    stream = asStream(input);
    expected = { { "D", 4 }, { "C", 3 }, { "B", 2 }, { "A", 1 } };
    EXPECT_EQUAL(topK(stream, 5), expected);
}

PROVIDED_TEST("topK: stream many elements, ask for top 1") {
    stringstream stream = asStream(1, kMany);
    Vector<DataPoint> expected = { { "" , kMany } };
    EXPECT_EQUAL(topK(stream, 1), expected);
}

PROVIDED_TEST("topK: stream many elements, ask for top 5") {
    stringstream stream = asStream(1, kMany);
    Vector<DataPoint> expected = {
        { "" , kMany     },
        { "" , kMany - 1 },
        { "" , kMany - 2 },
        { "" , kMany - 3 },
        { "" , kMany - 4 },
    };
    EXPECT_EQUAL(topK(stream, 5), expected);
}

PROVIDED_TEST("topK: stress test, many elements, random values") {
    Vector<double> expected;
    Vector<DataPoint> points;
    fillVector(points, 10000);
    for (DataPoint dp : points) {
        expected.add(dp.priority);
    }
    sort(expected.begin(), expected.end(), greater<double>());

    stringstream stream = asStream(points);
    int k = 10;
    Vector<DataPoint> result = topK(stream, k);
    EXPECT_EQUAL(result.size(), k);
    for (int i = 0; i < k; i++) {
        EXPECT_EQUAL(result[i].priority, expected[i]);
    }
}

PROVIDED_TEST("topK: stress test - many elements, ask for top half") {
    stringstream stream = asStream(1, kMany);
    Vector<DataPoint> result = topK(stream, kMany/2);
    EXPECT_EQUAL(result.size(), kMany/2);
    EXPECT_EQUAL(result[0].priority, kMany);
    EXPECT_EQUAL(result[result.size()-1].priority, kMany - result.size() + 1);
}

PROVIDED_TEST("topK: time trial") {
    int startSize = 200000;
    int k = 10;
    for (int n = startSize; n < 10*startSize; n *= 2) {
        Vector<DataPoint> input;
        fillVector(input, n);
        stringstream stream = asStream(input);
        TIME_OPERATION(n, topK(stream, k));
    }
}

