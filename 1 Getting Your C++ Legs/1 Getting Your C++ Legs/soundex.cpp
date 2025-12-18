/*
 * TODO: remove and replace this file header comment
 * This is a .cpp file you will edit and turn in.
 * Remove starter comments and add your own
 * comments on each function and on complex code sections.
 */
#include <cctype>
#include <fstream>
#include <string>
#include "console.h"
#include "strlib.h"
#include "filelib.h"
#include "simpio.h"
#include "vector.h"
#include "SimpleTest.h" // IWYU pragma: keep (needed to quiet spurious warning)
using namespace std;

#include "hashmap.h"

/* This function is intended to return a string which
 * includes only the letter characters from the original
 * (all non-letter characters are excluded)
 *
 * WARNING: The provided code is buggy!
 *
 * Use test cases to identify which inputs to this function
 * are incorrectly handled. Then, remove this comment and
 * replace it with a description of the bug you fixed.
 */
string lettersOnly(string s) {
    string result = "";
    for (int i = 0; i < s.length(); i++) {
        if (isalpha(s[i])) {
            result += s[i];
        }
    }
    return result;
}

/* This function is intended take in a string reference
 * 'originalString' of only letters
 * and alter that reference after encoding it using
 * the dictionary provided to us by the instructions
 * (all letters corresponds to a number)
 * eg. "Law" returns "400"
 */
void encodeString(string& originalString) {
    string upperCaseString = toUpperCase(originalString);
    string encodedString = "";

    HashMap<string, string> code = {
        {"A", "0"}, {"E", "0"}, {"I", "0"}, {"O", "0"}, {"U", "0"}, {"H", "0"}, {"W", "0"}, {"Y", "0"},
        {"B", "1"}, {"F", "1"}, {"P", "1"}, {"V", "1"},
        {"C", "2"}, {"G", "2"}, {"J", "2"}, {"K", "2"}, {"Q", "2"}, {"S", "2"}, {"X", "2"}, {"Z", "2"},
        {"D", "3"}, {"T", "3"},
        {"L", "4"},
        {"M", "5"}, {"N", "5"},
        {"R", "6"}
    };

    for (char c: upperCaseString) {
        encodedString += code.get(charToString(c));
    }

    originalString = encodedString;
}

/* This function is intended to take in a string reference
 * 'digitString' of only numbers 0-6
 * and remove any duplicates numbers that come one after another
 * eg. "22200299" returns "2029"
 */
void removeDuplicates(string& digitString) {
    string duplicatesRemoved = "";
    string previousLetter = "";

    for (char c: digitString) {
        string currentLetter = charToString(c);
        if (currentLetter != previousLetter) {
            duplicatesRemoved += currentLetter;
            previousLetter = currentLetter;
        }
    }

    digitString = duplicatesRemoved;
}

/* This function is intended to take in two parameters
 * one being a string representing the original string,
 * 'originalString', and the other being a reference to a string of all digits,
 * 'digitString', with no duplicates and it alters the reference
 * string to take on the same starting letter as the original string
 * eg. passing in "'Law", "40" returns "L0"
 */
void replaceFirstDigit(string originalString, string& digitString) {
    if (digitString.size() > 0) {
        string initialWord = lettersOnly(originalString);
        digitString[0] = toUpperCase(initialWord[0]);
    }
}

/* This function is intended to take in a string reference,
 * 'digitString', of all digits except for the first which is a
 * capitalized letter a-z and remove all the zeros from the string
 * eg. "A520400" returns "A524"
 */
void discardZeros(string& digitString) {
    string discardedZeros = "";
    int stringLength = digitString.size();

    for (int i = 0; i < stringLength; i++) {
        if (charToString(digitString[i]) != "0") {
            discardedZeros += digitString[i];
        }
    }

    digitString = discardedZeros;
}

/* This function is intended to take in a string reference of all digits
 * except the first which is a capitalized letter a-z, 'digitString',
 * and add zeros to the end of the string as long as it is under 4 characters long
 * or remove digits from the end of the string if it is over 4 characters long
 * eg. "L4" returns "L400"
 */
void paddingZeros(string& digitString) {
    while (digitString.size() < 4) {
        digitString += "0";
    }
}

/* This function is intended to take a string of all digits
 * except the first which is a capitalized letter a-z, 'digitString',
 * and remove any charcter after the 4th index.
 * eg. L12534 returns L125
 */
void truncatingCode(string& digitString) {
    if (digitString.size() > 4) {
        digitString = digitString.substr(0, 4);
    }
}

/* This function is intended to take in a string representing a surname,
 * 's', and encode it into a four-character string in the
 * form of and initial capital letter followed by three digits
 * then return that encoded string
 * eg. Law returns L000
 */
string soundex(string s) {
    /* TODO: Fill in this function. */
    string encoded = lettersOnly(s);
    encodeString(encoded);
    removeDuplicates(encoded);
    replaceFirstDigit(s, encoded);
    discardZeros(encoded);
    paddingZeros(encoded);
    truncatingCode(encoded);

    return encoded;
}


/* This function is intended to take in a string, 'filepath'.
 * The program repeatedly prompts the user to enter a surname
 * to look up in the file, calculating for the soundex code for that surname
 * and printing out all the surnames in the file with the same soundex code
 * using the 'soundex' function. It prompts the user to fill out different names
 * until they hit return which exits the function.
 */
void soundexSearch(string filepath) {
    // This provided code opens the specified file
    // and reads the lines into a vector of strings
    ifstream in;
    Vector<string> allNames;

    if (openFile(in, filepath)) {
        allNames = readLines(in);
    }
    cout << "Read file " << filepath << ", "
         << allNames.size() << " names found." << endl;

    // The names read from file are now stored in Vector allNames

    /* TODO: Fill in the remainder of this function. */
    int allNamesLength = allNames.size();

    while (true) {
        string surname = getLine("Enter a surname (RETURN to quit): ");

        if (surname.empty()) {
            cout << "All done!" << endl;
            break;
        }

        string encodedSurname = soundex(surname);
        cout << "Soundex code is " << encodedSurname << endl;

        Vector<string> matchingCodes;

        for (int i = 0; i < allNamesLength; i++) {
            string uniqueName = allNames[i];
            if (soundex(uniqueName) == encodedSurname) {
                matchingCodes.add(uniqueName);
            }
        }

        matchingCodes.sort();
        cout << "Matches from database: " << matchingCodes << "\n" << endl;
    }
}


/* * * * * * Test Cases * * * * * */


PROVIDED_TEST("Test exclude of punctuation, digits, and spaces") {
    string s = "O'Hara";
    string result = lettersOnly(s);
    EXPECT_EQUAL(result, "OHara");
    s = "Planet9";
    result = lettersOnly(s);
    EXPECT_EQUAL(result, "Planet");
    s = "tl dr";
    result = lettersOnly(s);
    EXPECT_EQUAL(result, "tldr");
}


PROVIDED_TEST("Sample inputs from handout") {
    EXPECT_EQUAL(soundex("Curie"), "C600");
    EXPECT_EQUAL(soundex("O'Conner"), "O256");
}

PROVIDED_TEST("hanrahan is in lowercase") {
    EXPECT_EQUAL(soundex("hanrahan"), "H565");
}

PROVIDED_TEST("DRELL is in uppercase") {
    EXPECT_EQUAL(soundex("DRELL"), "D640");
}

PROVIDED_TEST("Liu has to be padded with zeros") {
    EXPECT_EQUAL(soundex("Liu"), "L000");
}

PROVIDED_TEST("Tessier-Lavigne has a hyphen") {
    EXPECT_EQUAL(soundex("Tessier-Lavigne"), "T264");
}

PROVIDED_TEST("Au consists of only vowels") {
    EXPECT_EQUAL(soundex("Au"), "A000");
}

PROVIDED_TEST("Egilsdottir is long and starts with a vowel") {
    EXPECT_EQUAL(soundex("Egilsdottir"), "E242");
}

PROVIDED_TEST("Jackson has three adjcaent duplicate codes") {
    EXPECT_EQUAL(soundex("Jackson"), "J250");
}

PROVIDED_TEST("Schwarz begins with a pair of duplicate codes") {
    EXPECT_EQUAL(soundex("Schwarz"), "S620");
}

PROVIDED_TEST("Van Niekerk has a space between repeated n's") {
    EXPECT_EQUAL(soundex("Van Niekerk"), "V526");
}

PROVIDED_TEST("Wharton begins with Wh") {
    EXPECT_EQUAL(soundex("Wharton"), "W635");
}

PROVIDED_TEST("Ashcraft is not a special case") {
    // Some versions of Soundex make special case for consecutive codes split by hw
    // We do not make this special case, just treat same as codes split by vowel
    EXPECT_EQUAL(soundex("Ashcraft"), "A226");
}

// TODO: add your test cases here

STUDENT_TEST("testing the 'lettersOnly' function to see if it works for strings starting with symbols, strings including symbols, strings that are all letters,  strings that are all symbols, and empty string") {
    EXPECT_EQUAL(lettersOnly("!jlaw"), "jlaw");
    EXPECT_EQUAL(lettersOnly("O'Conner"), "OConner");
    EXPECT_EQUAL(lettersOnly("law"), "law");
    EXPECT_EQUAL(lettersOnly("!*(())"), "");
    EXPECT_EQUAL(lettersOnly("2"), "");
}

STUDENT_TEST("testing that my 'encodeString' function works for normal cases, just a vowel, a consonant, and an empty string") {
    string surname = "Curie";
    encodeString(surname);
    EXPECT_EQUAL(surname, "20600");

    surname = "OConner";
    encodeString(surname);
    EXPECT_EQUAL(surname, "0205506");

    surname = "A";
    encodeString(surname);
    EXPECT_EQUAL(surname, "0");

    surname = "B";
    encodeString(surname);
    EXPECT_EQUAL(surname, "1");

    surname = "";
    encodeString(surname);
    EXPECT_EQUAL(surname, "");
}

STUDENT_TEST("testing that my 'removeCoalesce' function works for normal cases, strings with all the same numbers, and an empty string") {
    string surnameDigits = "20600";
    removeDuplicates(surnameDigits);
    EXPECT_EQUAL(surnameDigits, "2060");

    surnameDigits = "0205506";
    removeDuplicates(surnameDigits);
    EXPECT_EQUAL(surnameDigits, "020506");

    surnameDigits = "1111111";
    removeDuplicates(surnameDigits);
    EXPECT_EQUAL(surnameDigits, "1");

    surnameDigits = "";
    removeDuplicates(surnameDigits);
    EXPECT_EQUAL(surnameDigits, "");
}

STUDENT_TEST("testing that my 'replaceFirstDigit' function works for all normal cases, strings startig with symbols, strings with symbols, and the empty string") {
    string surname = "Curie";
    string surnameDigits = "2060";
    replaceFirstDigit(surname, surnameDigits);
    EXPECT_EQUAL(surnameDigits, "C060");

    surname = "'Law";
    surnameDigits = "40";
    replaceFirstDigit(surname, surnameDigits);
    EXPECT_EQUAL(surnameDigits, "L0");

    surname = "O'Conner";
    surnameDigits = "020506";
    replaceFirstDigit(surname, surnameDigits);
    EXPECT_EQUAL(surnameDigits, "O20506");

    surname = "";
    surnameDigits = "";
    replaceFirstDigit(surname, surnameDigits);
    EXPECT_EQUAL(surnameDigits, "");
}

STUDENT_TEST("testing that my 'discardZeros' function works for all normal cases and the empty string") {
    string surnameDigits = "C060";
    discardZeros(surnameDigits);
    EXPECT_EQUAL(surnameDigits, "C6");

    surnameDigits = "O20506";
    discardZeros(surnameDigits);
    EXPECT_EQUAL(surnameDigits, "O256");

    surnameDigits = "";
    discardZeros(surnameDigits);
    EXPECT_EQUAL(surnameDigits, "");
}

STUDENT_TEST("testing that my 'paddingZeros' function works for strings with less than 4 characters, strings with exactly 4 characters, strings with more than 4 characters, and the empty string") {
    string surnameDigits = "C6";
    paddingZeros(surnameDigits);
    EXPECT_EQUAL(surnameDigits, "C600");

    surnameDigits = "O256";
    paddingZeros(surnameDigits);
    EXPECT_EQUAL(surnameDigits, "O256");

    surnameDigits = "O25634";
    paddingZeros(surnameDigits);
    EXPECT_EQUAL(surnameDigits, "O25634");

    surnameDigits = "";
    paddingZeros(surnameDigits);
    EXPECT_EQUAL(surnameDigits, "0000");
}

STUDENT_TEST("testing that my 'truncatingCode' function works for strings with more than 4 characters, strings with less than 4 characters, and the empty string") {
    string surnameDigits = "C6452123";
    truncatingCode(surnameDigits);
    EXPECT_EQUAL(surnameDigits, "C645");

    surnameDigits = "W5";
    truncatingCode(surnameDigits);
    EXPECT_EQUAL(surnameDigits, "W5");

    surnameDigits = "";
    truncatingCode(surnameDigits);
    EXPECT_EQUAL(surnameDigits, "");
}

STUDENT_TEST("testing my 'soundex' function for all different types of cases") {
    EXPECT_EQUAL(soundex("Curie"), "C600");
    EXPECT_EQUAL(soundex("PeppaPig'in'InsideOut"), "P112");
    EXPECT_EQUAL(soundex("!j33"), "J000");
}
