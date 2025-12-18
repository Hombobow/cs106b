/*
 *  This file contains the implementation of the Tile class
 */

#include "Tile.h"
#include "SimpleTest.h"
#include "direction.h"

using namespace std;

Tile::Tile(string n, string e, string s, string w) {
    _id = n + e + s + w;    // sets an id for internal use, do not change this line!

    //initializing the member variables
    _north = n;
    _east = e;
    _south = s;
    _west = w;
}

string Tile::getEdge(Direction dir) const {
    //returns the label associated with the direction on the tile
    switch (dir) {
        case NORTH: return _north;
        case EAST: return _east;
        case SOUTH: return _south;
        case WEST: return _west;
    }
}

void Tile::rotate() {
    string temp = _east; //string to hold onto east
    //flip each one
    _east = _north;
    _north = _west;
    _west = _south;
    _south = temp;
}


/* The functions below have already been written for you! */

Tile::Tile() : Tile("","","","") {}    // default constructor creates blank tile

bool Tile::isBlank() const {
    return (_north == "" && _east == "" && _south == "" && _west == "");
}

string Tile::displayTileStr() const{
    string s;
    s += "  " + _north + "\n";
    s += _west;
    s += "   " + _east + "\n";
    s += "  " + _south + "\n";
    return s;
}

string Tile::toString() const {
    return _north + "-" + _east + "-" + _south + "-" + _west;
}


/* * * * * * Test Cases * * * * * */

PROVIDED_TEST("Testing Tile constructor") {
    Tile t1("red", "green", "blue", "yellow");
    EXPECT_EQUAL(t1.toString(), "red-green-blue-yellow");
}

PROVIDED_TEST("Testing Tile::getEdge()") {
    Tile t1("A", "B", "C", "D");
    EXPECT_EQUAL(t1.getEdge(NORTH), "A");
    EXPECT_EQUAL(t1.getEdge(EAST), "B");
    EXPECT_EQUAL(t1.getEdge(SOUTH), "C");
    EXPECT_EQUAL(t1.getEdge(WEST), "D");
}

PROVIDED_TEST("Testing Tile::rotate()") {
    // Test single rotate
    Tile t1("A", "B", "C", "D");
    t1.rotate();
    EXPECT_EQUAL(t1.getEdge(NORTH), "D");
    EXPECT_EQUAL(t1.getEdge(EAST), "A");
    EXPECT_EQUAL(t1.toString(), "D-A-B-C");
    t1.rotate();
    EXPECT_EQUAL(t1.toString(), "C-D-A-B");

    // Test rotate full circle
    Tile t2("E", "F", "G", "H");
    string orig = t2.toString();
    for (int i = 0; i < NUM_SIDES; i++)
        t2.rotate();
    EXPECT_EQUAL(t2.toString(), orig);

    // Test rotations greater than a full circle
    Tile t3("1", "2", "3", "4");
    for (int i = 0; i < 6; i++)
        t3.rotate();
    EXPECT_EQUAL(t3.toString(), "3-4-1-2");
}

PROVIDED_TEST("Testing blank Tile") {
    Tile empty;
    EXPECT(empty.isBlank());
    EXPECT_EQUAL(empty.getEdge(NORTH), "");
    EXPECT_EQUAL(empty.toString(), "---");
    empty.rotate();
    EXPECT_EQUAL(empty.toString(), "---");
}
