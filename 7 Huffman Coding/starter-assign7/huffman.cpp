#include "bits.h"
#include "treenode.h"
#include "huffman.h"
#include "map.h"
#include "vector.h"
#include "priorityqueue.h"
#include "strlib.h"
#include "SimpleTest.h"  // IWYU pragma: keep (needed to quiet spurious warning)
#include <cmath> //for the extension
using namespace std;



/**
 * Given a Queue<Bit> containing the compressed message bits and the encoding tree
 * used to encode those bits, decode the bits back to the original message text.
 *
 * You can assume that tree is a well-formed non-empty encoding tree and
 * messageBits queue contains a valid sequence of encoded bits.
 *
 * Your implementation may change the messageBits queue however you like. There
 * are no requirements about what it should look like after this function
 * returns. The encoding tree should be unchanged.
 *
 * Iterates over the messageBits decoding character by character.
 */
string decodeText(EncodingTreeNode* tree, Queue<Bit>& messageBits) {
    //declare a string to store the decodedText
    string decodedText = "";

    //dequeue until messageBits is out of bits
    while(!messageBits.isEmpty()) {
        //start at the top of the tree
        EncodingTreeNode* node = tree;

        //traverse the tree until we find a character
        while (!node->isLeaf()) {
            if (messageBits.dequeue() == 0) {
                node = node->zero;
            } else {
                node = node->one;
            }
        }

        //append the leaf to the decoded text
        decodedText += node->getChar();
    }

    return decodedText;
}

/**
 * Reconstruct encoding tree from flattened form Queue<Bit> and Queue<char>.
 *
 * You can assume that the queues are well-formed and represent
 * a valid encoding tree.
 *
 * Your implementation may change the queue parameters however you like. There
 * are no requirements about what they should look like after this function
 * returns.
 *
 * Uses depth first recursion to reconstruct the full tree with the base case
 * being when we reach a leaf.
 */
EncodingTreeNode* unflattenTree(Queue<Bit>& treeShape, Queue<char>& treeLeaves) {
    //base case
    if (treeShape.dequeue() == 0) {
        //return a leaf node with the next dequeued character from treeLeaves
        return new EncodingTreeNode(treeLeaves.dequeue());
    }

    //recursive case
    EncodingTreeNode* left = unflattenTree(treeShape, treeLeaves);
    EncodingTreeNode* right = unflattenTree(treeShape, treeLeaves);

    //return the combination of the left and right subtrees
    return new EncodingTreeNode(left, right);
}

/**
 * Decompress the given EncodedData and return the original text.
 *
 * You can assume the input data is well-formed and was created by a correct
 * implementation of compress.
 *
 * Your implementation may change the data parameter however you like. There
 * are no requirements about what it should look like after this function
 * returns.
 *
 * Uses both the unflattenTree function and the decode text function to fully decode
 * an encoded piece of data given through EncodedData.
 */
string decompress(EncodedData& data) {
    //create the encoding tree using the unflattenTree function
    EncodingTreeNode* tree = unflattenTree(data.treeShape, data.treeLeaves);

    //store the decoded text in a string using the decodeText function
    string decodedText = decodeText(tree, data.messageBits);

    //deallocate the memory of the tree
    deallocateTree(tree);

    return decodedText;
}

/**
 * Constructs an optimal Huffman coding tree for the given text, using
 * the algorithm described in lecture.
 *
 * Reports an error if the input text does not contain at least
 * two distinct characters.
 *
 * When assembling larger trees out of smaller ones, make sure to set the first
 * tree dequeued from the queue to be the zero subtree of the new tree and the
 * second tree as the one subtree.
 *
 * Creates a huffman tree from a string of text for encoding purposes through the use of
 * both a map and a priority queue.
 *
 * Iterates over the priority queue of node pointers with priorities equal to frequency
 * concatenating the first two until and enqueuing it back in based on the combined priority
 * until there is one node left and returning that node (which is the full huffman tree).
 *
 * This huffman tree implementation breaks ties by dequeuing the elements in the reverse
 * order in which they were enqueued.
 */
EncodingTreeNode* buildHuffmanTree(string text) {
    //create a new map that tracks the number of occurences of each character
    Map<char, int> occurences;

    //loop through each character
    for (char c: text) {
        occurences[c]++;
    }

    //check if there is at least 2 characters
    if (occurences.size() < 2) {
        error("Must have at least 2 characters");
    }

    //initialize a new priority queue to store the huffman encoding tree
    PriorityQueue<EncodingTreeNode*> huffman;

    //loop through the map and add each character leaf with a priority equal to its frequency
    for (char c: occurences) {
        EncodingTreeNode* leaf = new EncodingTreeNode(c);
        huffman.enqueue(leaf, occurences[c]);
    }

    //while the priority queue has more than one element
    //dequeue the first two elements and create a new interior node with the two elements as its children
    while (huffman.size() > 1) {
        //dequeue and store the first two elements along with their priorities
        int childOnePriortiy = huffman.peekPriority();
        EncodingTreeNode* childOne = huffman.dequeue();

        int childTwoPriority = huffman.peekPriority();
        EncodingTreeNode* childTwo = huffman.dequeue();

        //create a new interior node to insert into the priority queue
        EncodingTreeNode* newNode = new EncodingTreeNode(childOne, childTwo);

        //enqueue the node in the priority queue
        huffman.enqueue(newNode, childOnePriortiy + childTwoPriority);
    }

    return huffman.dequeue();
}

//helper function that takes the encoding tree, a reference to a path vector of bits,
//a reference to a map of characters corresponding to vectors of bits and fills the map
//with the corresponding character and encodings
void buildMapHelper(EncodingTreeNode* tree, Vector<Bit>& path, Map<char, Vector<Bit>>& map) {
    //base case
    if (tree->isLeaf()) {
        map[tree->getChar()] = path;
        return;
    }

    //recursive left

    //choose
    path.add(0);
    //explore
    buildMapHelper(tree->zero, path, map);
    //unchoose
    path.remove(path.size() - 1);

    //recursive right

    //choose
    path.add(1);
    //explore
    buildMapHelper(tree->one, path, map);
    //unchoose
    path.remove(path.size() - 1);
}

/**
 * Given a string and an encoding tree, encode the text using the tree
 * and return a Queue<Bit> of the encoded bit sequence.
 *
 * You can assume tree is a valid non-empty encoding tree and contains an
 * encoding for every character in the text.
 *
 * Uses a helper funciton to create a map that returns the huffman encoding for each
 * character and iterates over the characters of the text, appending each huffman encoding
 * into a Queue of bits as it goes.
 */
Queue<Bit> encodeText(EncodingTreeNode* tree, string text) {
    //create a new map to store the huffman encoding tree
    Map<char, Vector<Bit>> encodingMap;

    //create a new path
    Vector<Bit> path;

    //build the map using the helper function
    buildMapHelper(tree, path, encodingMap);

    //create a new Queue of bits to store the sequence to return
    Queue<Bit> sequence;

    //iterate through the characters in the text
    for (char c: text) {
        //create a vector of the encoding for the corresponding character
        Vector<Bit> encoding = encodingMap[c];

        //iterate through the vector
        for (Bit b: encoding) {
            sequence.enqueue(b);
        }
    }

    return sequence;
}

/**
 * Flatten the given tree into a Queue<Bit> and Queue<char> in the manner
 * specified in the assignment writeup.
 *
 * You can assume the input queues are empty on entry to this function.
 *
 * You can assume tree is a valid well-formed encoding tree.
 *
 * Uses depth first recursion to flatten the tree with finding a leaf being the base case.
 */
void flattenTree(EncodingTreeNode* tree, Queue<Bit>& treeShape, Queue<char>& treeLeaves) {
    //base case
    if (tree->isLeaf()) {
        treeShape.enqueue(0);
        treeLeaves.enqueue(tree->getChar());
        return;
    }

    //recursive case
    treeShape.enqueue(1);
    flattenTree(tree->zero, treeShape, treeLeaves);
    flattenTree(tree->one, treeShape, treeLeaves);
    return;
}

/**
 * Compress the input text using Huffman coding, producing as output
 * an EncodedData containing the encoded message and flattened
 * encoding tree used.
 *
 * Reports an error if the message text does not contain at least
 * two distinct characters.
 *
 * Uses the functions buildHuffmanTree, encodeText, and flattenTree to compress a given
 * message into EncodedData.
 */
EncodedData compress(string messageText) {
    //build a huffman tree for the message
    //the function reports an error if the message does not contain at least two distinct characters
    EncodingTreeNode* huffmanTree = buildHuffmanTree(messageText);

    //encode the text
    Queue<Bit> encodedText = encodeText(huffmanTree, messageText);

    //encode the tree
    Queue<Bit> treeShape;
    Queue<char> treeLeaves;
    flattenTree(huffmanTree, treeShape, treeLeaves);

    //create a new encoded data to return
    EncodedData compressed;
    compressed.messageBits = encodedText;
    compressed.treeShape = treeShape;
    compressed.treeLeaves = treeLeaves;

    //deallocate the huffman tree
    deallocateTree(huffmanTree);

    return compressed;
}

/* * * * * * Testing Helper Functions Below This Point * * * * * */

EncodingTreeNode* createExampleTree() {
    /* Example encoding tree used in multiple test cases:
     *                *
     *              /   \
     *             T     *
     *                  / \
     *                 *   E
     *                / \
     *               R   S
     */

    //create the leaf nodes
    EncodingTreeNode* T = new EncodingTreeNode('T');
    EncodingTreeNode* S = new EncodingTreeNode('S');
    EncodingTreeNode* R = new EncodingTreeNode('R');
    EncodingTreeNode* E = new EncodingTreeNode('E');

    //create the various interior nodes
    EncodingTreeNode* RS = new EncodingTreeNode(R, S);
    EncodingTreeNode* RSE = new EncodingTreeNode(RS, E);

    //create the root node
    EncodingTreeNode* root = new EncodingTreeNode(T, RSE);

    return root;
}

void deallocateTree(EncodingTreeNode* t) {
    //base case
    if (t == nullptr) {
        return;
    }

    //recursive case
    deallocateTree(t->zero);
    deallocateTree(t->one);

    //delete the node
    delete t;
}

bool areEqual(EncodingTreeNode* a, EncodingTreeNode* b) {
    //base case
    if (a == nullptr && b == nullptr) {
        return true;
    }

    if (a == nullptr || b == nullptr) {
        return false;
    }

    if (a->isLeaf() && b->isLeaf()) {
        return a->getChar() == b->getChar();
    }

    if (a->isLeaf() != b->isLeaf()) {
        return false;
    }

    //recursive case
    return areEqual(a->zero, b->zero) && areEqual(a->one, b->one);
}

//extension Shannon entropy
double computeShannonEntropy(const Map<char,int>& occurences) {

    //create a new variable to store the total
    double total = 0;

    //loop through the frequency map and sum up the total
    for (char c: occurences) {
        total += occurences[c];
    }

    //create a new variable to store the shannon entropy
    double entropy = 0;

    //sum up according to the shannon entropy equation
    for (char c: occurences) {
        double probabilityOfC = occurences[c] / total;
        entropy += -probabilityOfC * log2(probabilityOfC);
    }

    return entropy;
}

/* * * * * * Test Cases Below This Point * * * * * */

//warmup student tests for createExampleTree

STUDENT_TEST("using createExampleTree to allocate a tree and then deallocateTree") {
    //creating an example tree
    EncodingTreeNode* exampleTree = createExampleTree();

    //deallocating the tree
    deallocateTree(exampleTree);
}

//warmup student tests for areEqual

STUDENT_TEST("Two empty trees should compare as true according to areEqual.") {
    EncodingTreeNode* emptyTreeOne = nullptr;
    EncodingTreeNode* emptyTreeTwo = nullptr;

    EXPECT(areEqual(emptyTreeOne, emptyTreeTwo));
}

STUDENT_TEST("The simplest non-empty encoding tree vs. empty tree") {
    EncodingTreeNode* leafOne = new EncodingTreeNode('A');
    EncodingTreeNode* leafTwo = new EncodingTreeNode('B');

    EncodingTreeNode* simpleTree = new EncodingTreeNode(leafOne, leafTwo);
    EncodingTreeNode* emptyTree = nullptr;

    EXPECT(!areEqual(simpleTree, emptyTree));

    deallocateTree(simpleTree);
}

STUDENT_TEST("The simplest non-empty encoding tree vs. another simple tree with children swapped") {
    EncodingTreeNode* leafOne = new EncodingTreeNode('A');
    EncodingTreeNode* leafTwo = new EncodingTreeNode('B');

    EncodingTreeNode* simpleTreeOne = new EncodingTreeNode(leafOne, leafTwo);

    EncodingTreeNode* leafThree = new EncodingTreeNode('A');
    EncodingTreeNode* leafFour = new EncodingTreeNode('B');
    EncodingTreeNode* simpleTreeTwo = new EncodingTreeNode(leafFour, leafThree);

    EXPECT(!areEqual(simpleTreeOne, simpleTreeTwo));

    deallocateTree(simpleTreeOne);
    deallocateTree(simpleTreeTwo);
}

STUDENT_TEST("The simplest non-empty encoding tree vs. another simple tree with different children") {
    EncodingTreeNode* leafOne = new EncodingTreeNode('A');
    EncodingTreeNode* leafTwo = new EncodingTreeNode('B');

    EncodingTreeNode* simpleTreeOne = new EncodingTreeNode(leafOne, leafTwo);

    EncodingTreeNode* leafThree = new EncodingTreeNode('C');
    EncodingTreeNode* leafFour = new EncodingTreeNode('D');
    EncodingTreeNode* simpleTreeTwo = new EncodingTreeNode(leafThree, leafFour);

    EXPECT(!areEqual(simpleTreeOne, simpleTreeTwo));

    deallocateTree(simpleTreeOne);
    deallocateTree(simpleTreeTwo);
}

STUDENT_TEST("The simplest non-empty encoding tree vs. an example tree") {
    EncodingTreeNode* leafOne = new EncodingTreeNode('A');
    EncodingTreeNode* leafTwo = new EncodingTreeNode('B');

    EncodingTreeNode* simpleTree = new EncodingTreeNode(leafOne, leafTwo);

    EncodingTreeNode* exampleTree = createExampleTree();

    EXPECT(!areEqual(simpleTree, exampleTree));

    deallocateTree(simpleTree);
    deallocateTree(exampleTree);
}

STUDENT_TEST("example tree vs. a subtree of the example tree") {
    EncodingTreeNode* exampleTree = createExampleTree();

    EXPECT(!areEqual(exampleTree, exampleTree->one));

    deallocateTree(exampleTree);
}

//assignment student tests for decodeText

STUDENT_TEST("decodeText, two leafed tree") {
    EncodingTreeNode* leafOne = new EncodingTreeNode('A');
    EncodingTreeNode* leafTwo = new EncodingTreeNode('B');

    EncodingTreeNode* tree = new EncodingTreeNode(leafOne, leafTwo);

    Queue<Bit> messageBits = { 0, 0, 1, 1 }; //AABB
    EXPECT_EQUAL(decodeText(tree, messageBits), "AABB");

    messageBits = { 1, 0, 0, 1, 0, 0 }; //BAABAA
    EXPECT_EQUAL(decodeText(tree, messageBits), "BAABAA");

    deallocateTree(tree);
}

STUDENT_TEST("decodeText, four leafed tree") {
    EncodingTreeNode* leafOne = new EncodingTreeNode('G');
    EncodingTreeNode* leafTwo = new EncodingTreeNode('O');
    EncodingTreeNode* leafThree = new EncodingTreeNode('A');
    EncodingTreeNode* leafFour = new EncodingTreeNode('T');

    EncodingTreeNode* GO = new EncodingTreeNode(leafOne, leafTwo);
    EncodingTreeNode* GOA = new EncodingTreeNode(GO, leafThree);
    EncodingTreeNode* tree = new EncodingTreeNode(GOA, leafFour);

    Queue<Bit> messageBits = { 0, 1, 1 }; //AT
    EXPECT_EQUAL(decodeText(tree, messageBits), "AT");

    messageBits = { 0, 0, 1, 0, 1, 1 }; //OAT
    EXPECT_EQUAL(decodeText(tree, messageBits), "OAT");

    messageBits = { 0, 0, 0, 0, 0, 1, 0, 1, 1 }; //GOAT
    EXPECT_EQUAL(decodeText(tree, messageBits), "GOAT");

    messageBits = { 1, 0, 1, 0, 0, 0 }; //TAG
    EXPECT_EQUAL(decodeText(tree, messageBits), "TAG");


    deallocateTree(tree);
}

//assignment student tests for decompress

STUDENT_TEST("testing decompress on a two-character tree") {
    EncodedData data;
    data.treeShape = { 1, 0, 0 };
    data.treeLeaves = { 'A', 'B' };
    data.messageBits = { 0, 1, 1, 0 };

    string output = decompress(data);
    EXPECT_EQUAL(output, "ABBA");
}

STUDENT_TEST("testing decompress on a six character tree") {
    EncodedData data;
    data.treeShape = { 1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0 };
    data.treeLeaves = { 'G', 'O', 'A', 'T', 'H', 'E' };
    data.messageBits = { 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0 };
    //THEGOAT

    string output = decompress(data);
    EXPECT_EQUAL(output, "THEGOAT");
}

//assignment student tests for encodeText

STUDENT_TEST("testing if decodeText undoes encodeText") {
    EncodingTreeNode* exampleTree = createExampleTree(); // see diagram above

    Queue<Bit> encodedSequence = encodeText(exampleTree, "TRES");
    string decodedText = decodeText(exampleTree, encodedSequence);
    EXPECT_EQUAL(decodedText, "TRES");

    encodedSequence = encodeText(exampleTree, "REST");
    decodedText = decodeText(exampleTree, encodedSequence);
    EXPECT_EQUAL(decodedText, "REST");

    deallocateTree(exampleTree);
}

//assignment student tests for flattenTree

STUDENT_TEST("testing if unflattenTree undoes flattenTree for example tree") {
    EncodingTreeNode* exampleTree = createExampleTree(); // see diagram above

    Queue<Bit> treeShape;
    Queue<char> treeLeaves;

    flattenTree(exampleTree, treeShape, treeLeaves);

    EncodingTreeNode* unflattenedTree = unflattenTree(treeShape, treeLeaves);

    EXPECT(areEqual(exampleTree, unflattenedTree));

    deallocateTree(exampleTree);
    deallocateTree(unflattenedTree);
}

STUDENT_TEST("testing if unflattenTree undoes flattenTree for GOAT tree") {
    EncodingTreeNode* leafOne = new EncodingTreeNode('G');
    EncodingTreeNode* leafTwo = new EncodingTreeNode('O');
    EncodingTreeNode* leafThree = new EncodingTreeNode('A');
    EncodingTreeNode* leafFour = new EncodingTreeNode('T');

    EncodingTreeNode* GO = new EncodingTreeNode(leafOne, leafTwo);
    EncodingTreeNode* GOA = new EncodingTreeNode(GO, leafThree);
    EncodingTreeNode* tree = new EncodingTreeNode(GOA, leafFour);

    Queue<Bit> treeShape;
    Queue<char> treeLeaves;

    flattenTree(tree, treeShape, treeLeaves);

    EncodingTreeNode* unflattenedTree = unflattenTree(treeShape, treeLeaves);

    EXPECT(areEqual(tree, unflattenedTree));

    deallocateTree(tree);
    deallocateTree(unflattenedTree);
}

//assignment student tests for buildHuffmanTree

STUDENT_TEST("testing if buildHuffman Tree calls an error on an input of only one character type") {
    EXPECT_ERROR(buildHuffmanTree("AAAAAAAA"));
}

STUDENT_TEST("testing if buildHuffman Tree correctly creates a tree with two types of characters") {
    EncodingTreeNode* huffmanTree = buildHuffmanTree("ABBB");

    EncodingTreeNode* A = new EncodingTreeNode('A');
    EncodingTreeNode* B = new EncodingTreeNode('B');
    EncodingTreeNode* solution = new EncodingTreeNode(A, B);

    EXPECT(areEqual(huffmanTree, solution));

    deallocateTree(huffmanTree);
    deallocateTree(solution);
}

STUDENT_TEST("testing if buildHuffman Tree correctly dequeues the latest enqueued element to break ties") {
    EncodingTreeNode* huffmanTree = buildHuffmanTree("ABCD");

    EncodingTreeNode* A = new EncodingTreeNode('A');
    EncodingTreeNode* B = new EncodingTreeNode('B');
    EncodingTreeNode* C = new EncodingTreeNode('C');
    EncodingTreeNode* D = new EncodingTreeNode('D');
    EncodingTreeNode* DC = new EncodingTreeNode(D, C);
    EncodingTreeNode* BA = new EncodingTreeNode(B, A);
    EncodingTreeNode* solution = new EncodingTreeNode(BA, DC);

    EXPECT(areEqual(huffmanTree, solution));

    deallocateTree(huffmanTree);
    deallocateTree(solution);
}

//assignment student tests for compress


STUDENT_TEST("testing the compress function using decompress on one example") {
    string word = "testing on a long STRING of words to see if my decompress function undoes my compress function";

    EncodedData input = compress(word);
    string output = decompress(input);

    EXPECT_EQUAL(word, output);
}

STUDENT_TEST("testing the compress function using decompress on many examples") {
    Vector<string> inputs = {
        "The goat",
        "The goat the goat the goat the goat the goat ate at eat there",
        "Spaceship tommorrow hello world today present popcorn brownie",
    };

    for (string input: inputs) {
        EncodedData data = compress(input);
        string output = decompress(data);

        EXPECT_EQUAL(input, output);
    }
}

//extension student test for shannon entropy

STUDENT_TEST("testing if the theoretical lower bound from shannon entropy is lower than the huffman encoding for a repeating text") {
    //store the text in a string
    string text = "TESTING TESTING TESTING";

    //compute the frequency table for the text
    Map<char, int> occurences;
    for (char c: text) {
        occurences[c]++;
    }

    //compute the shannon entropy and calculate the theoretical lower limit
    double entropy = computeShannonEntropy(occurences);
    double theoreticalMinBits = entropy * text.length();

    //encode the text through huffman encoding
    EncodedData encoded = compress(text);
    int huffmanBits = encoded.messageBits.size();

    //print out the text
    cout << "text: " << text << endl;
    cout << "shannon entropy (bits per char): " << entropy << endl;
    cout << "theoretical minimum bits: " << theoreticalMinBits << endl;
    cout << "huffman bits: " << huffmanBits << endl;

    EXPECT((int) theoreticalMinBits <= huffmanBits);
}

STUDENT_TEST("testing if the theoretical lower bound from shannon entropy is lower than the huffman encoding for long text") {
    //store the text in a string
    string text = "HELLOWORLDTODAYISAGOODDAYITISTHANKSGIVING!";

    //compute the frequency table for the text
    Map<char, int> occurences;
    for (char c: text) {
        occurences[c]++;
    }

    //compute the shannon entropy and calculate the theoretical lower limit
    double entropy = computeShannonEntropy(occurences);
    double theoreticalMinBits = entropy * text.length();

    //encode the text through huffman encoding
    EncodedData encoded = compress(text);
    int huffmanBits = encoded.messageBits.size();

    //print out the text
    cout << "text: " << text << endl;
    cout << "shannon entropy (bits per char): " << entropy << endl;
    cout << "theoretical minimum bits: " << theoreticalMinBits << endl;
    cout << "huffman bits: " << huffmanBits << endl;

    EXPECT((int) theoreticalMinBits <= huffmanBits);
}

/* * * * * Provided Tests Below This Point * * * * */

PROVIDED_TEST("decodeText, small example encoding tree") {
    EncodingTreeNode* tree = createExampleTree(); // see diagram above
    EXPECT(tree != nullptr);

    Queue<Bit> messageBits = { 1, 1 }; // E
    EXPECT_EQUAL(decodeText(tree, messageBits), "E");

    messageBits = { 1, 0, 1, 1, 1, 0 }; // SET
    EXPECT_EQUAL(decodeText(tree, messageBits), "SET");

    messageBits = { 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1}; // STREETS
    EXPECT_EQUAL(decodeText(tree, messageBits), "STREETS");

    deallocateTree(tree);
}

PROVIDED_TEST("unflattenTree, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above
    Queue<Bit>  treeShape  = { 1, 0, 1, 1, 0, 0, 0 };
    Queue<char> treeLeaves = { 'T', 'R', 'S', 'E' };
    EncodingTreeNode* tree = unflattenTree(treeShape, treeLeaves);

    EXPECT(areEqual(tree, reference));

    deallocateTree(tree);
    deallocateTree(reference);
}

PROVIDED_TEST("decompress, small example input") {
    EncodedData data = {
        { 1, 0, 1, 1, 0, 0, 0 }, // treeShape
        { 'T', 'R', 'S', 'E' },  // treeLeaves
        { 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1 } // messageBits
    };

    EXPECT_EQUAL(decompress(data), "TRESS");
}

PROVIDED_TEST("buildHuffmanTree, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above
    EncodingTreeNode* tree = buildHuffmanTree("STREETTEST");
    EXPECT(areEqual(tree, reference));

    deallocateTree(reference);
    deallocateTree(tree);
}

PROVIDED_TEST("encodeText, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above

    Queue<Bit> messageBits = { 1, 1 }; // E
    EXPECT_EQUAL(encodeText(reference, "E"), messageBits);

    messageBits = { 1, 0, 1, 1, 1, 0 }; // SET
    EXPECT_EQUAL(encodeText(reference, "SET"), messageBits);

    messageBits = { 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1 }; // STREETS
    EXPECT_EQUAL(encodeText(reference, "STREETS"), messageBits);

    deallocateTree(reference);
}

PROVIDED_TEST("flattenTree, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above
    Queue<Bit>  expectedShape  = { 1, 0, 1, 1, 0, 0, 0 };
    Queue<char> expectedLeaves = { 'T', 'R', 'S', 'E' };

    Queue<Bit>  treeShape;
    Queue<char> treeLeaves;
    flattenTree(reference, treeShape, treeLeaves);

    EXPECT_EQUAL(treeShape,  expectedShape);
    EXPECT_EQUAL(treeLeaves, expectedLeaves);

    deallocateTree(reference);
}

PROVIDED_TEST("compress, small example input") {
    EncodedData data = compress("STREETTEST");
    Queue<Bit>  treeShape   = { 1, 0, 1, 1, 0, 0, 0 };
    Queue<char> treeChars   = { 'T', 'R', 'S', 'E' };
    Queue<Bit>  messageBits = { 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0 };

    EXPECT_EQUAL(data.treeShape, treeShape);
    EXPECT_EQUAL(data.treeLeaves, treeChars);
    EXPECT_EQUAL(data.messageBits, messageBits);
}

PROVIDED_TEST("Test end-to-end compress -> decompress") {
    Vector<string> inputs = {
        "HAPPY HIP HOP",
        "Nana Nana Nana Nana Nana Nana Nana Nana Batman",
        "Research is formalized curiosity. It is poking and prying with a purpose. – Zora Neale Hurston",
    };

    for (string input: inputs) {
        EncodedData data = compress(input);
        string output = decompress(data);

        EXPECT_EQUAL(input, output);
    }
}
