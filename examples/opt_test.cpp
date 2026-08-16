int constantTest() {
    int a = 10 + 20 * 3; // Folded to 70 at compile-time by Rosie Optimizer (-O1/-O2)
    int b = a * 1 + 0;   // Algebraic simplification -> a
    return b;            // Dead code elimination will remove instructions after return
    int dead = 999;
}

int main() {
    return constantTest(); // returns 70
}
