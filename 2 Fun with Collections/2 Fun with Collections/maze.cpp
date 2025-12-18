#include <iostream>
#include <fstream>
#include "error.h"
#include "filelib.h"
#include "grid.h"
#include "maze.h"
#include "mazegraphics.h"
#include "queue.h"
#include "set.h"
#include "stack.h"
#include "vector.h"
#include "SimpleTest.h" // IWYU pragma: keep (needed to quiet spurious warning)
using namespace std;


// Given a maze of boolean values and a current GridLocation cur,
// the function returns a Set of all valid moves from cur.
// Valid moves include those that are in one of the four cardinal directions,
// within the bounds of the grid, and an open corridor

// I implemented this using 4 if statements coresponding to
// a GridLocation 4 cardinal directions and added a GridLocation if it met
// the criteria set above checking if it is in bounds first to not accidentally
// index into an out of bounds location
Set<GridLocation> generateValidMoves(Grid<bool>& maze, GridLocation cur) {
    Set<GridLocation> neighbors;

    int row = cur.row; // get current row
    int col = cur.col; // get current col

    if (maze.inBounds(row + 1, col) && maze[row + 1][col]) { // check if the south is valid move
        neighbors.add({row + 1, col});
    }
    if (maze.inBounds(row - 1, col) && maze[row - 1][col]) { // check if north is a valid move
        neighbors.add({row - 1, col});
    }
    if (maze.inBounds(row, col + 1) && maze[row][col + 1]) { // check if east is a valid move
        neighbors.add({row, col + 1});
    }
    if (maze.inBounds(row, col - 1) && maze[row][col - 1]) { // check if west is a valid move
        neighbors.add({row, col - 1});
    }

    return neighbors;
}

// Given a maze of booleans and a vector with GridLocations representing
// a path to complete the maze. This function raises an error if the path
// is not empty, the path doesn't start at the upper-left corner, the path
// doesn't finish at the lower-right corner, a location on a path is not a valid move
// or the path contains a loop

// I implemented this using different if statements that bring up errors
// if the condition is not met and for loops that iterate through the path
// checking for consitency with the requirements
void validatePath(Grid<bool>& maze, Vector<GridLocation>& path) {
    GridLocation start = {0, 0};
    GridLocation exit = { maze.numRows()-1, maze.numCols()-1 };

    if (path.isEmpty()) { // condition for if the path is empty
        error("Path is empty!");
    }

    if (path[0] != start) { // condition to check if the path doesn't start at {0, 0}
        error("Path doesn't start at the upper-left corner of the maze");
    }

    if (path[path.size() - 1] != exit) { // condition to check if the final location is the exit
        error("Path doesn't end at the lower-right corner of the maze");
    }

    for (int i = 0; i < path.size() - 1; i++) { // loop to make sure every suceeding path is valid
        // no moving into a wall or moving outside the maze using generateValidMoves helper function
        Set<GridLocation> validMoves = generateValidMoves(maze, path[i]);
        if (!(validMoves.contains(path[i + 1]))) {
            error("Invalid move was made");
        }
    }

    Set<GridLocation> seenMoves;

    for (int i = 0; i < path.size(); i++) { //looping through every move to check for a duplicate move (going in loops)
        if (seenMoves.contains(path[i])) {
            error("Duplicate move");
        }
        seenMoves.add(path[i]);
    }
}

// This function returns a vector of GridLocations that represents the solution
// using a breadth first search method where you visit the whole breadth on that level of
// search before trying the next depth

// I implemented this using a set of visited locations and iterating through
// the queue of paths until it is empty starting with the start vector {0, 0}
// and then adding paths using the generateValidMoves function to queue more moves
// into a new set of paths if the last move of the path
// is not the exit. To ensure there is no loops I made sure to ignore any cases where the
// generation of valid moves gave me soemthing in my visited locations set.
// Because I used a queue the FIFO function gives me each breadth before the next depth

Vector<GridLocation> solveMazeBFS(Grid<bool>& maze) {
    Vector<GridLocation> path;
    Queue<Vector<GridLocation>> allPaths;
    Set<GridLocation> visitedPaths;
    GridLocation exit = {maze.numRows() - 1, maze.numCols() - 1};

    drawMaze(maze);

    allPaths.enqueue({{0, 0}}); //starting path
    while (!(allPaths.isEmpty())) { //loop until I have exhuasted all paths
        path = allPaths.dequeue();
        highlightPath(path, "green", 3);
        int lastIndex = path.size() - 1;

        visitedPaths.add(path[lastIndex]);

        if (path[lastIndex] == exit) {
            return path;
        }
        else {
            //loop to add all viable next paths.
            for (GridLocation neighbor: generateValidMoves(maze, path[lastIndex])) {
                if (visitedPaths.contains(neighbor)) {
                    continue;
                }
                Vector<GridLocation> nextPath = path;
                nextPath.add(neighbor);
                allPaths.enqueue(nextPath);
            }
        }
    }

    return path; //assuming all paths has a solution as per the assignment instructions
    //this line of code should never run
}

// This function returns a vector of GridLocations that represents the solution
// using a depth first search method where you go deep into one path until you try another

// I implemented this by changing the original Queue that I into a stack that utilizes that
// LIFO function and changing all instances of enqueue and dequeue to push and pop. The LIFO
// model allows me to dive all the way down one path until i visit the next using a stack
// to attach my paths.
Vector<GridLocation> solveMazeDFS(Grid<bool>& maze) {
    Vector<GridLocation> path;
    Stack<Vector<GridLocation>> allPaths;
    Set<GridLocation> visitedPaths;
    GridLocation exit = {maze.numRows() - 1, maze.numCols() - 1};

    drawMaze(maze);

    allPaths.push({{0, 0}}); //starting path
    while (!(allPaths.isEmpty())) { //loop until I have exhuasted all paths
        path = allPaths.pop();
        highlightPath(path, "green", 3);
        int lastIndex = path.size() - 1;

        visitedPaths.add(path[lastIndex]);

        if (path[lastIndex] == exit) {
            return path;
        }
        else {
            //loop to add all viable next paths.
            for (GridLocation neighbor: generateValidMoves(maze, path[lastIndex])) {
                if (visitedPaths.contains(neighbor)) {
                    continue;
                }
                Vector<GridLocation> nextPath = path;
                nextPath.add(neighbor);
                allPaths.push(nextPath);
            }
        }
    }

    return path; //assuming all paths has a solution as per the assignment instructions
    //this line of code should never run
}

/*
 * The given readMazeFile function correctly reads a well-formed
 * maze from a file.
 *
 * This provided function is fully correct. You do not need to change
 * any of the code in this function.
 */
void readMazeFile(string filename, Grid<bool>& maze) {
    /* The following code reads data from the file into a Vector
     * of strings representing the lines of the file.
     */
    ifstream in;

    if (!openFile(in, filename))
        error("Cannot open file named " + filename);

    Vector<string> lines = stringSplit(readEntire(in), '\n');

    /* Now that the file data has been read into the Vector, populate
     * the maze grid.
     */
    int numRows = lines.size();        // rows is count of lines
    int numCols = lines[0].length();   // cols is length of line
    maze.resize(numRows, numCols);     // resize grid dimensions

    for (int r = 0; r < numRows; r++) {
        if (lines[r].length() != numCols) {
            error("Maze row has inconsistent number of columns");
        }
        for (int c = 0; c < numCols; c++) {
            char ch = lines[r][c];
            if (ch == '@') {        // wall
                maze[r][c] = false;
            } else if (ch == '-') { // corridor
                maze[r][c] = true;
            } else {
                error("Maze location has invalid character: '" + charToString(ch) + "'");
            }
        }
    }
}

/* The given readSolutionFile correctly reads a path from a file.
 *
 * This provided function is fully correct. You do not need to change
 * any of the code in this function.
 */
void readSolutionFile(string filename, Vector<GridLocation>& soln) {
    ifstream in;

    if (!openFile(in, filename)) {
        error("Cannot open file named " + filename);
    }

    if (!(in >> soln)) {// if not successfully read
        error("Maze solution did not have the correct format.");
    }
}


/* * * * * * Test Cases * * * * * */

PROVIDED_TEST("generateValidMoves on location in the center of 3x3 grid with no walls") {
    Grid<bool> maze = {{true, true, true},
                       {true, true, true},
                       {true, true, true}};
    GridLocation center = {1, 1};
    Set<GridLocation> expected = {{0, 1}, {1, 0}, {1, 2}, {2, 1}};

    EXPECT_EQUAL(generateValidMoves(maze, center), expected);
}

PROVIDED_TEST("generateValidMoves on location on the side of 3x3 grid with no walls") {
    Grid<bool> maze = {{true, true, true},
                       {true, true, true},
                       {true, true, true}};
    GridLocation side = {0, 1};
    Set<GridLocation> expected = {{0, 0}, {0, 2}, {1, 1}};

    EXPECT_EQUAL(generateValidMoves(maze, side), expected);
}

PROVIDED_TEST("generateValidMoves on corner of 2x2 grid with walls") {
    Grid<bool> maze = {{true, false},
                       {true, true}};
    GridLocation corner = {0, 0};
    Set<GridLocation> expected = {{1, 0}};

    EXPECT_EQUAL(generateValidMoves(maze, corner), expected);
}

PROVIDED_TEST("validatePath on correct solution, hand-constructed maze") {
    Grid<bool> maze = {{true, false},
                       {true, true}};
    Vector<GridLocation> soln = { {0 ,0}, {1, 0}, {1, 1} };

    EXPECT_NO_ERROR(validatePath(maze, soln));
}

PROVIDED_TEST("validatePath on correct solution read from file, medium maze") {
    Grid<bool> maze;
    Vector<GridLocation> soln;
    readMazeFile("res/5x7.maze", maze);
    readSolutionFile("res/5x7.soln", soln);

    EXPECT_NO_ERROR(validatePath(maze, soln));
}

PROVIDED_TEST("validatePath on invalid path should raise error") {
    Grid<bool> maze = {{true, false},
                       {true, true},
                       {true, true}};
    Vector<GridLocation> notBeginAtEntry = { {1, 1}, {2, 1} };
    Vector<GridLocation> notEndAtExit = { {0, 0}, {1, 0}, {2, 0} };
    Vector<GridLocation> moveThroughWall = { {0 ,0}, {0, 1}, {1, 1}, {2, 1} };

    EXPECT_ERROR(validatePath(maze, notBeginAtEntry));
    EXPECT_ERROR(validatePath(maze, notEndAtExit));
    EXPECT_ERROR(validatePath(maze, moveThroughWall));
}

PROVIDED_TEST("solveMazeBFS on file 5x7") {
    Grid<bool> maze;
    readMazeFile("res/5x7.maze", maze);
    Vector<GridLocation> soln = solveMazeBFS(maze);

    EXPECT_NO_ERROR(validatePath(maze, soln));
}

PROVIDED_TEST("solveMazeDFS on file 21x23") {
    Grid<bool> maze;
    readMazeFile("res/21x23.maze", maze);
    Vector<GridLocation> soln = solveMazeDFS(maze);

    EXPECT_NO_ERROR(validatePath(maze, soln));
}

// generateValidMoves student tests
STUDENT_TEST("generateValidMoves on top-left corner with walls everywhere else") {
    Grid<bool> maze = {{true, false},
                       {false, false}};
    GridLocation topLeft = {0, 0};
    Set<GridLocation> expected = {};

    EXPECT_EQUAL(generateValidMoves(maze, topLeft), expected);
}

STUDENT_TEST("generateValidMoves when surrounded by walls") {
    Grid<bool> maze = {{true, false, true},
                       {false, true, false},
                       {true, false, true}};
    GridLocation center = {1, 1};
    Set<GridLocation> expected = {};

    EXPECT_EQUAL(generateValidMoves(maze, center), expected);
}

STUDENT_TEST("generateValidMoves on side with one path") {
    Grid<bool> maze = {{false, false, false},
                       {false, false, false},
                       {false, true, true}};
    GridLocation side = {2, 1};
    Set<GridLocation> expected = {{2, 2}};

    EXPECT_EQUAL(generateValidMoves(maze, side), expected);
}

// validatePath student tests
STUDENT_TEST("validatePath on invalid path should raise error") {
    Grid<bool> maze = {{true, true, true},
                       {false, true, false},
                       {true, true, true}};
    Vector<GridLocation> empty = {};
    Vector<GridLocation> loops = { {0, 0}, {0, 1}, {1, 1}, {2, 1}, {2, 0}, {2, 1}, {2, 2} };
    Vector<GridLocation> moveThroughWall = { {0, 0}, {1, 0}, {2, 0}, {2, 1}, {2,2} };

    EXPECT_ERROR(validatePath(maze, empty));
    EXPECT_ERROR(validatePath(maze, loops));
    EXPECT_ERROR(validatePath(maze, moveThroughWall));
}

STUDENT_TEST("validatePath on a 1x1 grid") {
    Grid<bool> maze = {{true}};
    Vector<GridLocation> solution = { {0,0} };
    Vector<GridLocation> invalidMove = { {0, 0}, {0, 0} };

    EXPECT_NO_ERROR(validatePath(maze, solution));
    EXPECT_ERROR(validatePath(maze, invalidMove));
}

// BFS student tests
STUDENT_TEST("BFS on a 19x35 maze") {
    Grid<bool> maze;
    Vector<GridLocation> solution;
    readMazeFile("res/19x35.maze", maze);
    readSolutionFile("res/19x35.soln", solution);
    EXPECT_EQUAL(solveMazeBFS(maze), solution);
}

STUDENT_TEST("BFS on a 25x33 maze") {
    Grid<bool> maze;
    Vector<GridLocation> solution;
    readMazeFile("res/25x33.maze", maze);
    readSolutionFile("res/25x33.soln", solution);
    EXPECT_EQUAL(solveMazeBFS(maze), solution);
}

// DFS student tests
STUDENT_TEST("DFS on a 19x35 maze") {
    Grid<bool> maze;
    Vector<GridLocation> solution;
    readMazeFile("res/19x35.maze", maze);
    readSolutionFile("res/19x35.soln", solution);
    EXPECT_EQUAL(solveMazeDFS(maze), solution);
}

STUDENT_TEST("DFS on a 25x33 maze") {
    Grid<bool> maze;
    Vector<GridLocation> solution;
    readMazeFile("res/25x33.maze", maze);
    readSolutionFile("res/25x33.soln", solution);
    EXPECT_EQUAL(solveMazeDFS(maze), solution);
}
