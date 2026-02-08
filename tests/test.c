// Test comment
/* Multiline
 * comment
 */
int main() {
    int x = -5;
    bool y = false;
    print("Hello, World!");
    print(y);
    y = true;
    print(y);
    if (!(x > 0)) {
        print("x is negative");
    }
    if (y) {
        print("y is true");
    } else {
        print("y is false");
        y = true;
        print(y);
        if (y) {
            print("y is now true");
        }
    }
    string z = "Woooow";
    print(z);
    while (x < 5) {
        x += 2;
        print(x);
    }
    print("X:");
    print(x);
    x -= 4;
    print(x);
    x *= 6;
    print(x);
    x /= 2;
    print(x);
    x++;
    print(x);
    x--;
    print(x);
    print("For loop test:");
    for (int i = 0; i <= 10; i += 1) {
        if (i == 5) {
            continue;
        }
        print(i);
        if (i == 8) {
            break;
        }
    }
    return 0;
}
