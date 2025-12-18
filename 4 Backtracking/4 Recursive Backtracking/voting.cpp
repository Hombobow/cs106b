/* This file has two functions computePowerIndexes which given a vector of blocks with different
 * voting sizes returns a vector with the corresponding banzhaf power indexes and computeCriticalVotes which is a helper function
 * computeCriticalVotes which takes a block size, a sum, an index, a n value representing a block, and a mjority count and returns
 * the amount of times the Nth block is a critical voter.
 */
#include <iostream>    // for cout, endl
#include <string>      // for string class
#include "voting.h"
#include "SimpleTest.h"
using namespace std;


int computeCriticalVotes(Vector<int>& blocks, int coalitionSum, int index, int& n, int& majority);

/* This function takes a N vector of blocks and returns another
 * N vector with the respective blocks Banzhaf Power Indexes
 */
Vector<int> computePowerIndexes(Vector<int>& blocks)
{
    Vector<int> result;
    int resultSize = blocks.size();
    int totalCriticalVotes = 0; //integer to store the total number of critical votes
    int majorityVotes = 0; //integer to store the majoirtyVotes

    //loop to count up the total votes
    for (int block: blocks) {
        majorityVotes += block;
    }

    //divide by 2 + 1for majority votes
    majorityVotes = majorityVotes / 2 + 1;

    //loop through the result vector
    for (int i = 0; i < resultSize; i++) {
        //grabbing the target block
        int target = blocks[i];
        int criticalVote;
        Vector<int> newBlock;

        //check if you are at the last index or the first index or in between
        if (i == resultSize - 1){
            newBlock = blocks.subList(0, resultSize - 1);
            criticalVote = computeCriticalVotes(newBlock, 0, 0, target, majorityVotes);
        } else {
            newBlock = blocks.subList(0, i) + blocks.subList(i + 1, resultSize - (i + 1));
            criticalVote = computeCriticalVotes(newBlock, 0, 0, target, majorityVotes);
        }

        result.add(criticalVote);

        //add the number of critical votes to the total critical votes
        totalCriticalVotes += criticalVote;
    }

    //finding the power index for each individual block by multiplying by 100 and dividing by totalCriticalVotes
    for (int j = 0; j < resultSize; j++) {
        result[j] = (result[j] * 100) / totalCriticalVotes; //compute banzhaf power index
    }

    return result;
}

/* This helper function takes a N-1 vector of blocks, and a coalitionSum (int), the index we are on (int), the final Nth block (int),
 * and a majority (int) and returns the number of times the Nth block is a critical voter.
 */
int computeCriticalVotes(Vector<int>& blocks, int coalitionSum, int index, int& n, int& majority) {
    //base case 1 for efficiency
    if (coalitionSum > majority) {
        return 0;
    }

    // base case 2
    if (index == blocks.size()) { //if the vector ever runs out of items that is one coalition
        // return 1 if the sum of the coalition is less than the majority and the sum + the nth block is greater than the majority
        return (coalitionSum < majority && (coalitionSum + n) >= majority);
    }

    //recursive case
    //this recurses through all coalitions excluding block n (int n) and adds up all the numbers the meet the base case
    return computeCriticalVotes(blocks, coalitionSum + blocks[index], index + 1, n, majority) +
           computeCriticalVotes(blocks, coalitionSum, index + 1, n, majority);
}


/* * * * * * Test Cases * * * * * */

PROVIDED_TEST("Test power index, blocks 50-49-1") {
    Vector<int> blocks = {50, 49, 1};
    Vector<int> expected = {60, 20, 20};
    EXPECT_EQUAL(computePowerIndexes(blocks), expected);
}

PROVIDED_TEST("Test power index, blocks Hempshead 1-1-3-7-9-9") {
    Vector<int> blocks = {1, 1, 3, 7, 9, 9};
    Vector<int> expected = {0, 0, 0, 33, 33, 33};
    EXPECT_EQUAL(computePowerIndexes(blocks), expected);
}

PROVIDED_TEST("Test power index, blocks CA-TX-NY 55-38-39") {
    Vector<int> blocks = {55, 38, 29};
    Vector<int> expected = {33, 33, 33};
    EXPECT_EQUAL(computePowerIndexes(blocks), expected);
}

PROVIDED_TEST("Test power index, blocks CA-TX-GA 55-38-16") {
    Vector<int> blocks = {55, 38, 16};
    Vector<int> expected = {100, 0, 0};
    EXPECT_EQUAL(computePowerIndexes(blocks), expected);
}

PROVIDED_TEST("Time power index operation") {
    Vector<int> blocks;
    for (int i = 0; i < 15; i++) {
        blocks.add(randomInteger(1, 10));
    }
    TIME_OPERATION(blocks.size(), computePowerIndexes(blocks));
}

PROVIDED_TEST("Test power index, blocks EU post-Nice") {
    // Estonia is one of those 4s!!
    Vector<int> blocks = {29,29,29,29,27,27,14,13,12,12,12,12,12,10,10,10,7,7,7,7,7,4,4,4,4,4,3};
    Vector<int> expected = {8, 8, 8, 8, 7, 7, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0};
    EXPECT_EQUAL(computePowerIndexes(blocks), expected);
}

//student test for computePowerIndexes
STUDENT_TEST("Test power index, blocks 60, 30, 20, 10, 5, 5") {
    Vector<int> blocks = {60, 30, 20, 10, 5, 5};
    Vector<int> expected = {63, 9, 9, 9, 4, 4};
    EXPECT_EQUAL(computePowerIndexes(blocks), expected);
}

//student tests for the computePowerIndexes funciton
STUDENT_TEST("testing power index for only one majority") {
    Vector<int> blocks = {6, 1, 1, 1, 1};
    Vector<int> expected = {100, 0, 0, 0, 0};
    EXPECT_EQUAL(computePowerIndexes(blocks), expected);
}

//fills a vector with n random blocks
void fillWithNBlocks(Vector<int>& blocks, int n) {
    for (int i = 0; i < n; i++) {
        blocks.add(randomInteger(1, 10));
    }
}

//scales the time of running a computePowerIndexes function by a factor of n
void scaleTimeByN(Vector<int> blocks, int n) {
    for (int i = 0; i < n; i++) {
        computePowerIndexes(blocks);
    }
}

STUDENT_TEST("testing the time for 5 different sizes") {
    Vector<int> blocks;

    for (int i = 3; i < 8; i++) {
        fillWithNBlocks(blocks, i);
        TIME_OPERATION(i, scaleTimeByN(blocks, 15));
    }
}
