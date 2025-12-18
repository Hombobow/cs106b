/* This file contains two functions countAllOrderings which returns the number possible orders of a and b and also countGoodOrderings
 * which returns all possible orders that are good which means A is leading throughout.
 */
#include "recursion.h"
#include "SimpleTest.h"
using namespace std;

/*
 * this function takes the two integers a and b and recursively counts all
 * combinations of possible tallies returning it as an int
 */
int countAllOrderings(int a, int b) {
    //base case
    if (a == 0 || b == 0) { //return a completed tally if one side runs out of tallies
        return 1;
    } else {
        // recursive cases
        return countAllOrderings(a - 1, b) +
               countAllOrderings(a, b - 1);
        //tallies an a in one recurse and tallies a b in another
    }
}

/*
 * this function assists countGoodOrderings in carrying out its functionality by
 * allowing it to abstract away a integer to count the lead
 */
int countGoodOrderingsHelper(int a, int b, int lead) {
    //base case
    if (lead < 1) {
        return 0;
    }
    if (a == 0 || b == 0) { //return a completed tally if one side runs out of tallies
        return 1;
    }

    // recursive cases
    return countGoodOrderingsHelper(a - 1, b, lead + 1) + countGoodOrderingsHelper(a, b - 1, lead - 1);
    //tallies an a in one recurse and tallies a b in another
}

/*
 * this function takes two integers a and b and recursively counts all
 * combinations of possible tallies that result in a leading throughout
 * the entire tally
 */
int countGoodOrderings(int a, int b) {
    if (a <= b) {
        return 0; //if you can't win there are no good orderings
    }

    if (a == 0) { //if you start with no As there are no good orderings
        return 0;
    }

    return countGoodOrderingsHelper(a - 1, b, 1); //tally an A first
}

/* * * * * * Test Cases * * * * * */

PROVIDED_TEST("countAllOrderings, two A one B") {
   EXPECT_EQUAL(countAllOrderings(2, 1), 3);
}

PROVIDED_TEST("countGoodOrderings, two A one B") {
   EXPECT_EQUAL(countGoodOrderings(2, 1), 1);
}

//student test cases for countAllOrderings
STUDENT_TEST("2 test cases for countAllOrderings") {
    EXPECT_EQUAL(countAllOrderings(1, 2), 3);
    EXPECT_EQUAL(countAllOrderings(2, 2), 6);
}

//student test cases for countGoodOrderings
STUDENT_TEST("2 test cases for countGoodOrderings") {
    EXPECT_EQUAL(countGoodOrderings(2, 1), 1);
    EXPECT_EQUAL(countGoodOrderings(4, 2), 5);
}

//student test cases for Bertrand’s Theorem
STUDENT_TEST("looping through different combinations of a and b") {
    for (int a = 5; a < 10; a++) {
        for (int b = 0; b < 5; b++) {
            double ratio = double(countGoodOrderings(a, b)) / countAllOrderings(a, b);
            double actualRatio = double(a - b) / (a + b);
            EXPECT_EQUAL(ratio, actualRatio);
        }
    }
}
