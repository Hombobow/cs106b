/*
 * TODO: remove and replace this file header comment
 * This is a .cpp file you will edit and turn in.
 * Remove starter comments and add your own
 * comments on each function and on complex code sections.
 */#include "console.h"
#include <iostream>
#include "SimpleTest.h" // IWYU pragma: keep (needed to quiet spurious warning)
using namespace std;

/* The divisorSum function takes one argument `n` and calculates the
 * sum of proper divisors of `n` excluding itself. To find divisors
 * a loop iterates over all numbers from 1 to n-1, testing for a
 * zero remainder from the division using the modulus operator %
 *
 * Note: the C++ long type is a variant of int that allows for a
 * larger range of values. For all intents and purposes, you can
 * treat it like you would an int.
 */
long divisorSum(long n) {
    long total = 0;
    for (long divisor = 1; divisor < n; divisor++) {
        if (n % divisor == 0) {
            total += divisor;
        }
    }
    return total;
}

/* The isPerfect function takes one argument `n` and returns a boolean
 * (true/false) value indicating whether or not `n` is perfect.
 * A perfect number is a non-zero positive number whose sum
 * of its proper divisors is equal to itself.
 */
bool isPerfect(long n) {
    return (n != 0) && (n == divisorSum(n));
}

/* The findPerfects function takes one argument `stop` and performs
 * an exhaustive search for perfect numbers over the range 1 to `stop`.
 * Each perfect number found is printed to the console.
 */
void findPerfects(long stop) {
    for (long num = 1; num < stop; num++) {
        if (isPerfect(num)) {
            cout << "Found perfect number: " << num << endl;
        }
        if (num % 10000 == 0) cout << "." << flush; // progress bar
    }
    cout << endl << "Done searching up to " << stop << endl;
}

/* The smarterSum function takes one argument 'n' and calculates the
 * sum of proper divisors of 'n' excluding itself. To find divisors
 * a loop iterates over all numbers from 1 to sqrt('n') inclusive, testing for a
 * zero remainder from the division using the modulus operator % and adding
 * the divisor and its pairwise factor.
 *
 * It also keeps in mind to only add the sqrt('n') divisor only once using a
 * conditional for perfect squares.
 */
long smarterSum(long n) {
    /* TODO: Fill in this function. */
    long total = 0;
    if (n == 1) {
        return 0; //1 has no divisors excluding itself
    }
    for (long divisor = 1; divisor <= sqrt(n); divisor++) {
        if (n % divisor == 0) {
            total += divisor;
            long pfactor = n/divisor;
            if (divisor != pfactor && divisor != 1) {
                total += pfactor;
            }
        }
    }
    return total;
}



/* The isPerfectSmarter function takes one argument 'n' and returns a boolean
 * (true/false) value indicating whether or not 'n' is perfect.
 * It determines this by checking that it is non zero and making sure that it
 * is equal to the sum of it's divisors using the 'smarterSum' function.
 */
bool isPerfectSmarter(long n) {
    /* TODO: Fill in this function. */
    return (n != 0) && (n == smarterSum(n));
}

/* The findPerfectsSmarter function takes one argument 'stop' and
 * searches for perfect numbers over the range 1 to 'stop'
 * using the 'isPerfectSmarter' function.
 * Each perfect number found is printed to the console.
 */
void findPerfectsSmarter(long stop) {
     /* TODO: Fill in this function. */
    for (int num = 1; num < stop; num++) {
         if (isPerfectSmarter(num)) {
             cout << "Found perfect number: " << num << endl;
         }
         if (num % 10000 == 0) cout << "." << flush; // progress bar
    }
}

/* The findNthPerfectEuclid function takes one argument 'n' and
 * searches for the nth perfect number by checking if the
 * Mersenne number (2^k-1) is prime. If it is the equation
 *  2^(k-1) * (2^k-1) is a perfect number. It loops until it has found
 *  'n' perfect number and returns it the nth one.
 */
long findNthPerfectEuclid(long n) {
    /* TODO: Fill in this function. */
    int k = 1;
    int i = 0;
    long pnum = 0;
    while (i < n) {
        long m = pow(2, k) - 1;
        if (divisorSum(m) == 1 && smarterSum(m) == 1) {
            pnum = (pow(2, k - 1)) * (pow(2, k) - 1);
            i++;
        }
        k++;
    }
    return pnum;
}


/* * * * * * Test Cases * * * * * */

/* Note: Do not add or remove any of the PROVIDED_TEST tests.
 * You should add your own STUDENT_TEST tests below the
 * provided tests.
 */

PROVIDED_TEST("Confirm 'divisorSum' of small inputs") {
    EXPECT_EQUAL(divisorSum(1), 0);
    EXPECT_EQUAL(divisorSum(6), 6);
    EXPECT_EQUAL(divisorSum(12), 16);
}

PROVIDED_TEST("Confirm 6 and 28 are perfect") {
    EXPECT(isPerfect(6));
    EXPECT(isPerfect(28));
}

PROVIDED_TEST("Confirm 12 and 98765 are not perfect") {
    EXPECT(!isPerfect(12));
    EXPECT(!isPerfect(98765));
}

PROVIDED_TEST("Test oddballs: 0 and 1 are not perfect") {
    EXPECT(!isPerfect(0));
    EXPECT(!isPerfect(1));
}

PROVIDED_TEST("Confirm 33550336 is perfect") {
    EXPECT(isPerfect(33550336));
}

PROVIDED_TEST("Time trial of 'findPerfects' on input size 1000") {
    TIME_OPERATION(1000, findPerfects(1000));
}


// TODO: add your student test cases here

STUDENT_TEST("experiment trail to find the largest size my computer can compute in ~60 seconds") {
    TIME_OPERATION(320000, findPerfects(320000));
}

STUDENT_TEST("four time trails of 'findPerfects', doubling in size each time") {
    int min = 40000;
    int max = 320000;

    for (int size = min; size <= max; size *= 2) {
        TIME_OPERATION(size, findPerfects(size));
    }
}

STUDENT_TEST("experiment trail to test the differences in time between computing 'isPerfect' on the numbers 10 and 1000") {
    TIME_OPERATION(10, isPerfect(10));
    TIME_OPERATION(1000, isPerfect(1000));
}

STUDENT_TEST("experiment trail to test the differences in time between computing 'findPerfects' on the numbers 1000 and 2000") {
    TIME_OPERATION(1000, findPerfects(1000));
    TIME_OPERATION(2000, findPerfects(2000));
}

STUDENT_TEST("testing the 'isPerfect' function on a few negative inputs to see if they result in false") {
    EXPECT(!isPerfect(-6));
    EXPECT(!isPerfect(-28));
    EXPECT(!isPerfect(-40000));
    EXPECT(!isPerfect(-320000));
}

STUDENT_TEST("testing the different case of 'smarterSum' to see if they align with divisorSum") {
    EXPECT_EQUAL(smarterSum(-1), divisorSum(-1));
    EXPECT_EQUAL(smarterSum(1), divisorSum(1));
    EXPECT_EQUAL(smarterSum(225), divisorSum(225));
    EXPECT_EQUAL(smarterSum(320000), divisorSum(320000));
    EXPECT_EQUAL(smarterSum(8589869056), divisorSum(8589869056));
}


STUDENT_TEST("four time trails of 'findPerfectsSmarter', doubling in size each time with the lagrgest size computed in ~60 seconds") {
    int min = 1280000;
    int max = 10240000;

    for (int size = min; size <= max; size *= 2) {
        TIME_OPERATION(size, findPerfectsSmarter(size));
    }
}

STUDENT_TEST("testing if 'findNthPerfectEuclid' returns the nth perfect number and seeing if any given number returned is perfect") {
    EXPECT_EQUAL(findNthPerfectEuclid(1), 6);
    EXPECT_EQUAL(findNthPerfectEuclid(6), 8589869056);
    EXPECT(isPerfectSmarter(findNthPerfectEuclid(1)));
    EXPECT(isPerfect(findNthPerfectEuclid(6)));
}


/*
 * Below is a suggestion of how to use a loop to set the input sizes
 * for a sequence of time trials.
 *
 *
STUDENT_TEST("Multiple time trials of findPerfects on increasing input sizes") {

    int smallest = 1000, largest = 8000;

    for (int size = smallest; size <= largest; size *= 2) {
        TIME_OPERATION(size, findPerfects(size));
    }
}

*/
