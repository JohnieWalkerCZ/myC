/*
 * A more complex test case for the language.
 */

// Recursive factorial function
int factorial(int n) {
    if (n == 0) {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

// Function to check for prime numbers
bool isPrime(int n) {
    if (n <= 1) {
        return 0; // false
    }
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) {
            return false; // false
        }
    }
    return true; // true
}

// Main function to test complex features
int main() {
    print("--- Complex Test Case ---");

    // Test factorial
    print("Factorial of 5:");
    int fact5 = factorial(5);
    print(fact5); // Expected: 120

    // Test prime numbers
    print("Prime numbers up to 30:");
    for (int i = 2; i <= 30; i++) {
        if (isPrime(i)) {
            print(i);
        }
    }

    // Nested loops and conditional statements
    print("Nested loops with conditions:");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (i == j) {
                print("i == j");
            } else if (i > j) {
                print("i > j");
            } else {
                print("i < j");
            }
        }
    }

    // String manipulation and concatenation (assuming '+' for concatenation)
    string s1 = "Hello";
    string s2 = ", ";
    string s3 = "World!";
    // string s4 = s1 + s2 + s3; // Assuming string concatenation
    // print(s4);

    print("--- End of Complex Test ---");

    return 0;
}
