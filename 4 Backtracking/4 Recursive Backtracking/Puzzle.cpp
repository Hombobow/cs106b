#include "Puzzle.h"
#include "PuzzleGUI.h"
#include "SimpleTest.h"

using namespace std;

bool Puzzle::canMatchEdge(Tile tile, GridLocation loc, Direction dir) const {
    //checking every direction
    if (dir == NORTH) {
        if (loc.row == 0) { //if we are on the top row return true
            return true;
        }
        loc.row -= 1; //check the tile above}
    } else if (dir == EAST) {
        if (loc.col == _grid.numCols() - 1) { //if we are on the rightmost column return true
            return true;
        }

        loc.col += 1; //check the tile to the right
    } else if (dir == SOUTH) {
        if (loc.row == _grid.numRows() - 1) { //if we are on the bottom row return true
            return true;
        }

        loc.row += 1; //check the tile below
    } else {
        if (loc.col == 0) { //if we are on the leftmost column return true
            return true;
        }

        loc.col -= 1; //check the tile to the left
    }

    if (tileAt(loc).isBlank()) { //return true if the adjacent tile is blank
        return true;
    }

    //if the adjacent tile is matching return true
    return isComplement(tile.getEdge(dir), tileAt(loc).getEdge(opposite(dir)));
}

/* Feel free to investigate the functions below, but you should not modify them */

// configures the puzzle with data read from the puzzle configuration file
void Puzzle::configure(int numRows, int numCols, Map<string, string>& pairs) {
    _complementMap = pairs;
    _grid.resize(numRows, numCols);
    _grid.clear();
    _numFilled = 0;
}

bool Puzzle::isFull() const {
    return _numFilled == _grid.size();
}

bool Puzzle::isEmpty() const {
    return _numFilled == 0;
}

bool Puzzle::canAdd(Tile tile) const {
    return !isFull() && canMatchAllEdges(tile, locationForCount(_numFilled));
}

void Puzzle::add(Tile tile) {
    if (isFull()) error("Cannot add to full grid!");
    GridLocation where = locationForCount(_numFilled);
    _grid[where] = tile;
    _numFilled++;
}

Tile Puzzle::remove() {
    if (isEmpty()) error("Cannot remove from empty grid!");
    GridLocation where = locationForCount(_numFilled - 1);
    Tile removed = _grid[where];
    _grid[where] = Tile(); // replace with blank tile
    _numFilled--;
    return removed;
}

// this is a little translation function to turn a 1-dimensional
// count into a 2-dimensional grid location
GridLocation Puzzle::locationForCount(int count) const {
    GridLocation loc;
    loc.row = count / _grid.numCols();
    loc.col = count % _grid.numCols();
    return loc;
}

// This is how we determine if two edges match (e.g., a red bottle top and a red bottle bottom)
// The map is populated with matching edge pairs read from the puzzle configuration file
bool Puzzle::isComplement(string one, string two) const {
    return _complementMap[one] == two;
}

//  verify each of the four edges of the tile matches its adjacent neighbor
bool Puzzle::canMatchAllEdges(Tile tile, GridLocation loc) const {
    for (Direction dir = NORTH; dir <= WEST; dir++) {
        if (!canMatchEdge(tile, loc, dir)) {
            return false;
        }
    }
    return true;
}

// access the tile at the given grid location
Tile Puzzle::tileAt(GridLocation loc) const {
    return _grid[loc];
}

// basic 2d traversal of the grid. Assumes the tile can print itself, as well
void Puzzle::print() const {
    GridLocation cur;
    for (cur.row = 0; cur.row < _grid.numRows(); cur.row++) {
        for (cur.col = 0; cur.col < _grid.numCols(); cur.col++) {
            cout << tileAt(cur) << "  ";
        }
        cout << endl;
    }
}


PROVIDED_TEST("Testing Puzzle::canMatchEdge()") {
    Map<string, string> pairs = {{"A","a"}, {"a", "A"}, {"B","b"}, {"b", "B"}, {"C","c"}, {"c","C"}};
    Puzzle puzzle;
    int numRows = 3, numCols = 2;
    puzzle.configure(numRows, numCols, pairs);

    Tile t1("A", "C", "b", "a");
    Tile t2("B", "a", "C", "c");
    Tile t3("B", "a", "b", "A");

    puzzle.add(t1);
    puzzle.add(t2);
    puzzle.add(t3);

    Tile t4("c", "B", "A", "c");

    // neighbor to right is out-of-bounds (guranteed match)
    EXPECT_EQUAL(puzzle.canMatchEdge(t4, {1,1}, EAST), true);

    // neighbor below is blank (guranteed match)
    EXPECT_EQUAL(puzzle.canMatchEdge(t4, {1,1}, SOUTH), true);

    // neighbor above is a match
    EXPECT_EQUAL(puzzle.canMatchEdge(t4, {1,1}, NORTH), true);

    // neightbor to left is NOT a match
    EXPECT_EQUAL(puzzle.canMatchEdge(t4, {1,1}, WEST), false);

    // after rotate, t4 will now match all edges
    t4.rotate();
    EXPECT_EQUAL(puzzle.canMatchEdge(t4, {1,1}, NORTH), true);
    EXPECT_EQUAL(puzzle.canMatchEdge(t4, {1,1}, EAST), true);
    EXPECT_EQUAL(puzzle.canMatchEdge(t4, {1,1}, SOUTH), true);
    EXPECT_EQUAL(puzzle.canMatchEdge(t4, {1,1}, WEST), true);

    // t4 should be valid to add
    EXPECT_EQUAL(puzzle.canAdd(t4), true);
}

