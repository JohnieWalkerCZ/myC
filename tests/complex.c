// test
/*
 * Super Test Case
 * Demonstrates: Algorithms, Sorting, Recursion, and Advanced Math
 */

// --- 1. Recursive Euclidean Algorithm for GCD ---
int gcd(int a, int b) {
    if (b == 0) {
        return a;
    }
    return gcd(b, a % b);
}

// --- 2. Recursive Fibonacci ---
int fibonacci(int n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

// --- 3. Prime Check Helper ---
bool isPrime(int n) {
    if (n <= 1)
        return false;
    for (int i = 2; i * i <= n; i = i + 1) {
        if (n % i == 0)
            return false;
    }
    return true;
}

int main() {
    print("=== STARTING SUPER TEST ===");

    // ---------------------------------------------------------
    // TEST 1: Advanced Recursion (Fibonacci)
    // ---------------------------------------------------------
    print("--- Fibonacci Sequence (First 10) ---");
    // Calculates: 0, 1, 1, 2, 3, 5, 8, 13, 21, 34
    for (int i = 0; i < 10; i = i + 1) {
        int f = fibonacci(i);
        print(f);
    }

    // ---------------------------------------------------------
    // TEST 2: Number Theory (GCD)
    // ---------------------------------------------------------
    print("--- GCD Calculation ---");
    int a = 105;
    int b = 252;
    // Should be 21
    print("GCD of 105 and 252 is:");
    print(gcd(a, b));

    // ---------------------------------------------------------
    // TEST 3: The Collatz Conjecture (While Loops)
    // ---------------------------------------------------------
    print("--- Collatz Sequence for n = 12 ---");
    // If n is even, divide by 2. If odd, multiply by 3 and add 1.
    // Ends when n is 1.
    int n = 12;
    while (n > 1) {
        print(n);
        if (n % 2 == 0) {
            n = n / 2;
        } else {
            n = 3 * n + 1;
        }
    }
    print(n); // Finally 1

    // ---------------------------------------------------------
    // TEST 4: Array Sorting (Bubble Sort)
    // ---------------------------------------------------------
    print("--- Bubble Sort Algorithm ---");

    int list[8];
    list[0] = 45;
    list[1] = 12;
    list[2] = 89;
    list[3] = 7;
    list[4] = 23;
    list[5] = 1;
    list[6] = 99;
    list[7] = 12; // Duplicate test
    print("Original array");
    for (int i = 0; i < 8; i++) {
        print(list[i]);
    }

    print("Sorting array...");

    int size = 8;
    // Bubble Sort: Swaps adjacent elements if they are in wrong order
    for (int i = 0; i < size - 1; i = i + 1) {
        for (int j = 0; j < size - i - 1; j = j + 1) {
            if (list[j] > list[j + 1]) {
                // Swap logic
                int temp = list[j];
                list[j] = list[j + 1];
                list[j + 1] = temp;
            }
        }
    }

    print("Sorted Result (Ascending):");
    for (int i = 0; i < size; i++) {
        print(list[i]);
    }

    // ---------------------------------------------------------
    // TEST 5: Inverse Search (Finding largest Prime)
    // ---------------------------------------------------------
    print("--- Largest Prime under 100 ---");
    // We iterate backwards. The first one we find is the answer.
    // Uses a flag variable to simulate a 'break'.
    int found = 0;
    for (int k = 100; k > 1; k = k - 1) {
        if (found == 0) {
            if (isPrime(k)) {
                print(k); // Should be 97
                found = 1;
            }
        }
    }

    print("=== TEST COMPLETE ===");
    return 0;
}
