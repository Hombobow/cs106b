/* This file contains 3 functions operatorsFrom which takes a string and removes everything but brackets, operatorsAreMatched which
 * makes sure there are no extraneous operators that dont have a pair, and isBalanced which just reports if a given string has balanced
 * bracketing
 */
#include <iostream>    // for cout, endl
#include <string>      // for string class
#include "recursion.h"
#include "SimpleTest.h"

using namespace std;

/*
 * this function takes a string str and recursively removes
 * any non bracketing characters and returns the string without them
 */
string operatorsFrom(string str) {
    //base case
    if (str.size() == 0) {
        return "";
    }

    string bracketing = "()[]{}"; //all the characters I want to keep

    //recursive case
    if (bracketing.find(str[0]) != string::npos) { //checking if the strings first character is brackets
        return str[0] + operatorsFrom(str.substr(1));
    }
    else {
        return operatorsFrom(str.substr(1));
    }
}

/* This function takes a string of only operators and
 * recursively removes brackets until there is only an empty string left
 * if that is possible it is balanced returning true
 * if not return false
 */
bool operatorsAreMatched(string ops) {
    //base case
    if (ops.size() == 0) {
        return true;
    }

    string curlyBraces = "{}";
    string squareBraces = "[]";
    string parenthesis = "()";

    //recursive cases
    if (ops.find(curlyBraces) != string::npos) {
        return operatorsAreMatched(ops.erase(ops.find(curlyBraces), 2));
    }
    if (ops.find(squareBraces) != string::npos) {
        return operatorsAreMatched(ops.erase(ops.find(squareBraces), 2));
    }
    if (ops.find(parenthesis) != string::npos) {
        return operatorsAreMatched(ops.erase(ops.find(parenthesis), 2));
    }

    return false;
}

/*
 * The isBalanced function assumes correct implementation of
 * the above two functions operatorsFrom and operatorsMatch.
 * It uses operatorsFrom to extract the operator characters
 * from the input string and then confirms that those
 * operators are balanced by using operatorsMatch.
 * You should not modify the provided code in the isBalanced
 * function.  If the previous two functions have been implemented
 * correctly, the provided isBalanced will correctly report whether
 * the input string has balanced bracketing operators.
 */
bool isBalanced(string str) {
    string ops = operatorsFrom(str);
    return operatorsAreMatched(ops);
}


/* * * * * * Test Cases * * * * * */

PROVIDED_TEST("operatorsFrom on simple example") {
    EXPECT_EQUAL(operatorsFrom("vec[3]"), "[]");
}

PROVIDED_TEST("operatorsAreMatched on simple example") {
    EXPECT(operatorsAreMatched("{}"));
}

PROVIDED_TEST("isBalanced on example from writeup") {
    string example ="int main() { int x = 2 * (vec[2] + 3); x = (1 + random()); }";
    EXPECT(isBalanced(example));
}

PROVIDED_TEST("isBalanced on non-balanced examples from writeup") {
    EXPECT(!isBalanced("( ( [ a ] )"));
    EXPECT(!isBalanced("3 ) ("));
    EXPECT(!isBalanced("{ ( x } y )"));
}

//student tests for operatorsFrom
STUDENT_TEST("operatorsFrom on 3 examples") {
    EXPECT_EQUAL(operatorsFrom("str.substr(0, 1)"), "()");
    EXPECT_EQUAL(operatorsFrom("function(int x, int y) {for(int i = 0; i < 10; i++)}"), "(){()}");
    EXPECT_EQUAL(operatorsFrom("{(hello world)}"), "{()}");
}

//student tests for operatorsAreMatched
STUDENT_TEST("operatorsAreMatched on 4 examples") {
    EXPECT(operatorsAreMatched("{()[]{}}"));
    EXPECT(!operatorsAreMatched("({(}))"));
    EXPECT(operatorsAreMatched("[[]{()[]{}}]"));
    EXPECT(!operatorsAreMatched("[[]{()]{}}]"));
}

//student tests for isBalanced
STUDENT_TEST("checking isBalanced with 2 test cases") {
    EXPECT(!isBalanced("()ss{s}s{s}s{[ss}s[s]s)s)s)"));
    EXPECT(!isBalanced("{ { x } y )"));
}
