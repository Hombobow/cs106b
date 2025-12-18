#include <iostream>
#include <fstream>
#include "error.h"
#include "filelib.h"
#include "map.h"
#include "search.h"
#include "set.h"
#include "simpio.h"
#include "strlib.h"
#include "vector.h"
#include "SimpleTest.h" // IWYU pragma: keep (needed to quiet spurious warning)
using namespace std;


// This function is intended to take a string and return
// the string with only letters and numbers and in all lowercase

// I implemented this behavior by running a loop through every
// character of the string in all lower case and saving only
// letters and digits
string cleanToken(string s) {
    string cleaned = ""; //string to store the cleaned up variable
    string lowerCase = toLowerCase(s); //lowercase copy of the variable

    for (char c: lowerCase) { //loop through every character
        if (isalpha(c) || isdigit(c)) { //conditions for appending to the string
            cleaned += c;
        }
    }

    return cleaned;
}

// This funciton is intended to take a string of text and return
// a set of strings that are cleanedTokens of every token in the string

// I implemented this behavior by creating a vector of tokens and looping
// through the tokens to clean them and add them to a set of cleaned tokens
// as long as they are not empty strings then returning the set
Set<string> gatherTokens(string text) {
    Vector<string> tokens = stringSplit(text, " "); //gathering a vector containing
    //all the strings in text seperate by " "
    Set<string> cleanedUniqueTokens; //used to store unique tokens

    for (string token: tokens) { //iterate through the strings
        if (cleanToken(token) != "") {
            cleanedUniqueTokens.add(cleanToken(token));
        }
    }

    return cleanedUniqueTokens;
}

// This function is intended to read through the content inside a database
// and create a inverted index that maps tokens to website links and return the
// number of website links processed

// I implemented this function by reading the file into a vector of strings representing
// each line that goes URL then tokens. Then I created a unique set of tokens with that iteration
// and mapped them to the website link makign sure to count that number of websites
// I iterated then added 1 to the count.
int buildIndex(string dbfile, Map<string, Set<string>>& index) {
    int count = 0; //stores the # of files processed
    ifstream in;

    if (!(openFile(in, dbfile))) {
        error("Cannot open file named " + dbfile);
    }
    Vector<string> lines = stringSplit(readEntire(in), '\n'); //reads the different lines in dbfile

    for (int i = 0; i < lines.size() - 1; i += 2) { //iterate through every 2 lines one for the url and one for the tokens
        Set<string> tokens = gatherTokens(lines[i + 1]); //set of unqiue tokens from each webpages site
        for (string token: tokens) {
            index[token] += lines[i];
        }
        count++; //adding to the processed files
    }

    return count;
}

// This function is intended to take a query and return a set of all possible search results

// I implemented this function by first creating a vector of the different unfiltered query tokens
// and checking the first character to either be +/-/nothing and creating a result based on that
// + corresponding to + two sets, - corresponds to - two sets, and anything else (nothing) corresponds
// to union with
Set<string> findQueryMatches(Map<string, Set<string>>& index, string query) {
    Set<string> result;
    Vector<string> tokens = stringSplit(query, " "); //a vector of the unfiltered query tokens with the +/-

    for (string token: tokens) { //loop through each token
        if (token[0] == '+') {
            result = result * index[cleanToken(token)]; // if the starting character is a + create a set that must match both terms
        }
        else if (token[0] == '-') {
            result = result - index[cleanToken(token)]; // if the starting character is a - create a set that matches one term and not the other
        }
        else {
            result.unionWith(index[cleanToken(token)]);
            // if the starting character is anything else create a set that any matching term from either set is included
            //this is always the first word
        }
    }

    return result;
}

// The searchEngine is designed to build an map of an inverted index given a database file
// and print out the related links according to the querys given by a user.

// I implemented this function by using a while loop to prompt the user for more
// queries until they return and for every query I use the inverted index created by
// running the data base through the buildIndex helper function to find all matching
// links to the query using a second helper function findQueryMatches then printing them out.
void searchEngine(string dbfile) {
    Map<string, Set<string>> index;
    Set<string> matches;
    string pages = integerToString(buildIndex(dbfile, index));

    cout << "Stand by while building index..." << endl;
    cout << "Indexed " << pages << " containing " << index.size() << " unique terms." << endl;

    while (true) { // always do this unless there is a break statement
        string query = getLine("Enter query sentence (RETURN/ENTER to quit): "); // prompt the user for a query
        if (query == "") { // condition for the break
            break;
        }

        matches = findQueryMatches(index, query); // use helper function findQueryMatches to find all corresponding links
        string numMatches = integerToString(matches.size());
        cout << "Found " << numMatches << " matching pages" << endl; // print them out
        cout << matches << endl;
        cout << endl;
    }
}

/* * * * * * Test Cases * * * * * */

PROVIDED_TEST("cleanToken on strings of letters and digits") {
    EXPECT_EQUAL(cleanToken("hello"), "hello");
    EXPECT_EQUAL(cleanToken("WORLD"), "world");
    EXPECT_EQUAL(cleanToken("CS106B"), "cs106b");
}

PROVIDED_TEST("cleanToken on strings containing punctuation") {
    EXPECT_EQUAL(cleanToken("/hello/"), "hello");
    EXPECT_EQUAL(cleanToken("~woRLD!"), "world");
    EXPECT_EQUAL(cleanToken("they're"), "theyre");
}

PROVIDED_TEST("cleanToken on string of only punctuation") {
    EXPECT_EQUAL(cleanToken("#$^@@.;"), "");
}

PROVIDED_TEST("gatherTokens from simple string") {
    Set<string> expected = {"go", "gophers"};
    EXPECT_EQUAL(gatherTokens("go go go gophers"), expected);
}

PROVIDED_TEST("gatherTokens correctly cleans tokens") {
    Set<string> expected = {"i", "love", "cs106b"};
    EXPECT_EQUAL(gatherTokens("I _love_ CS*106B!"), expected);
}

PROVIDED_TEST("gatherTokens from seuss, 6 unique tokens, mixed case, punctuation") {
    Set<string> tokens = gatherTokens("One Fish Two Fish *Red* fish Blue fish ** 10 RED Fish?");
    EXPECT_EQUAL(tokens.size(), 6);
    EXPECT(tokens.contains("fish"));
    EXPECT(!tokens.contains("Fish"));
}

PROVIDED_TEST("buildIndex from tiny.txt, 4 pages, 12 unique tokens") {
    Map<string, Set<string>> index;
    int nPages = buildIndex("res/tiny.txt", index);
    EXPECT_EQUAL(nPages, 4);
    EXPECT_EQUAL(index.size(), 12);
    EXPECT(index.containsKey("fish"));
}

PROVIDED_TEST("findQueryMatches from tiny.txt, single word query") {
    Map<string, Set<string>> index;
    buildIndex("res/tiny.txt", index);
    Set<string> matchesRed = findQueryMatches(index, "red");
    EXPECT_EQUAL(matchesRed.size(), 2);
    EXPECT(matchesRed.contains("www.dr.seuss.net"));
    Set<string> matchesHippo = findQueryMatches(index, "hippo");
    EXPECT(matchesHippo.isEmpty());
}

PROVIDED_TEST("findQueryMatches from tiny.txt, compound queries") {
    Map<string, Set<string>> index;
    buildIndex("res/tiny.txt", index);
    Set<string> matchesRedOrFish = findQueryMatches(index, "red fish");
    EXPECT_EQUAL(matchesRedOrFish.size(), 4);
    Set<string> matchesRedAndFish = findQueryMatches(index, "red +fish");
    EXPECT_EQUAL(matchesRedAndFish.size(), 1);
    Set<string> matchesRedWithoutFish = findQueryMatches(index, "red -fish");
    EXPECT_EQUAL(matchesRedWithoutFish.size(), 1);
}

// testing for cleanToken
STUDENT_TEST("cleanToken on different types of strings") {
    EXPECT_EQUAL(cleanToken(""), "");
    EXPECT_EQUAL(cleanToken("helloWorld!"), "helloworld");
    EXPECT_EQUAL(cleanToken("!jacky Law"), "jackylaw");
    EXPECT_EQUAL(cleanToken("70+bananaStacks"), "70bananastacks");
    EXPECT_EQUAL(cleanToken("Cs**106b!"), "cs106b");
}

// testing for gatherTokens
STUDENT_TEST("gatherTokens on different length strings") {
    Set<string> tokens = gatherTokens("helloWorld! 70+bananaStacks Cs**106b!");
    Set<string> expected = {"helloworld", "70bananastacks", "cs106b"};
    EXPECT_EQUAL(tokens, expected);

    tokens = gatherTokens("x x x x x y z a b c hello! *lol* !! *** lol! 0 1 2");
    expected = {"x", "y", "z", "a", "b", "c", "hello", "lol", "0", "1", "2"};
    EXPECT_EQUAL(tokens, expected);
}

STUDENT_TEST("gatherTokens on different length strings") {
    Set<string> tokens = gatherTokens("helloWorld! 70+bananaStacks Cs**106b!");
    Set<string> expected = {"helloworld", "70bananastacks", "cs106b"};
    EXPECT_EQUAL(tokens, expected);

    tokens = gatherTokens("x x x x x y z a b c hello! *lol* !! *** lol! 0 1 2");
    expected = {"x", "y", "z", "a", "b", "c", "hello", "lol", "0", "1", "2"};
    EXPECT_EQUAL(tokens, expected);
}

// testing for buildIndex including a time operation one
STUDENT_TEST("buildIndex on the tiny.txt") {
    string file = "res/tiny.txt";
    Map<string, Set<string>> index;
    int count = buildIndex(file, index);
    EXPECT_EQUAL(count, 4);
}

STUDENT_TEST("Use a time operation on buildIndex with parameter website.txt") {
    string file = "res/tiny.txt";
    Map<string, Set<string>> tinyIndex;
    TIME_OPERATION(12, buildIndex(file, tinyIndex));
    EXPECT_EQUAL(tinyIndex.size(), 12);

    file = "res/website.txt";
    Map<string, Set<string>> websiteIndex;
    TIME_OPERATION(4646, buildIndex(file, websiteIndex));
    EXPECT_EQUAL(websiteIndex.size(), 4646);
}

// test for findQueryMatches
STUDENT_TEST("findQueryMatches for different types of queries") {
    Map<string, Set<string>> index;
    buildIndex("res/tiny.txt", index);
    Set<string> myQuery = findQueryMatches(index, "fI**s?h +Bre(ad");
    EXPECT_EQUAL(myQuery.size(), 1);
    EXPECT_EQUAL(myQuery, {"www.shoppinglist.com"});

    myQuery = findQueryMatches(index, "fI**s?h +Bre(ad BL&ue");
    EXPECT_EQUAL(myQuery.size(), 3);
    EXPECT_EQUAL(myQuery, {"www.dr.seuss.net", "www.rainbow.org", "www.shoppinglist.com"});
}
