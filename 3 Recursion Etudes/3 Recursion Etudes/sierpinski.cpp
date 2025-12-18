/* This file has two functions fillBlackTriangle which fills a triangle defined by three points black
 * and drawSierpinskiTriangle which draws sierpinski triangles of any order
 */
#include <iostream>    // for cout, endl
#include "recursion.h"
#include "SimpleTest.h"
using namespace std;

/*
 * This function draws a filled, black triangle on the provided GWindow
 * that is defined by the corner GPoints one, two, and three. This
 * function has been provided for you and does not need to be modified.
 */
void fillBlackTriangle(GWindow& window, GPoint one, GPoint two, GPoint three) {
    window.setColor("black");
    window.fillPolygon( {one, two, three} );
}

/* This function takes three points and an order and draws
 * that order of the sierpinski triangle
 */
int drawSierpinskiTriangle(GWindow& window, GPoint one, GPoint two, GPoint three, int order) {
    //base case
    if (order == 0) {
        fillBlackTriangle(window, one, two, three);
        return 1;
    }

    GPoint oneTwo = {(double(one.x) + two.x) / 2, (double(one.y) + two.y) / 2}; //mp of one and two
    GPoint twoThree = {(double(two.x) + three.x) / 2, (double(two.y) + three.y) / 2}; //mp of two and three
    GPoint oneThree = {(double(one.x) + three.x) / 2, (double(one.y) + three.y) / 2}; //mp of one and three

    //recursive cases
    return drawSierpinskiTriangle(window, one, oneTwo, oneThree, order - 1) + //first triangle
           drawSierpinskiTriangle(window, oneTwo, two, twoThree, order - 1) + //second triangle
           drawSierpinskiTriangle(window, oneThree, twoThree, three, order - 1); // third triangle

    return 0;
}


/* * * * * * Test Cases * * * * * */

/*
 * Do not modify the code beyond this line! There are no
 * unit tests for this problem. You should do all your testing
 * via the interactive demo.
 */

PROVIDED_TEST("Test fractal drawing interactively using graphical demo") {
    runInteractiveDemo();
}

