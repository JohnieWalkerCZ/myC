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
            return false;
        }
    }
    return true;
}

void showMessage(string message) {
    print("---Message---");
    print(message);
}

int main() {
    print("--- Complex Test Case ---");
    int arr[5];
    for (int i = 0; i < 5; i++) {
        arr[i] = i;
    }

    for (int i = 0; i < 5; i++) {
        print(arr[i]);
    }

    showMessage("Array okay");

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

    print("--- End of Complex Test ---");

    return 0;
}
