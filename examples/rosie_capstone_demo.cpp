int compute(int x) {
    int result = 0;
    for (int i = 1; i <= x; i = i + 1) {
        result = result + i;
    }
    return result;
}

int categorize(int val) {
    int code = 0;
    switch (val) {
        case 10:
            code = 100;
            break;
        case 20:
            code = 200;
            break;
        default:
            code = 999;
            break;
    }
    return code;
}

int main() {
    int sum = compute(10); // 55
    int code1 = categorize(20); // 200
    int ternaryVal = (sum > 50) ? 1 : 0; // 1

    int count = 0;
    do {
        count = count + 1;
    } while (count < 5);

    return sum + code1 + ternaryVal + count; // 55 + 200 + 1 + 5 = 261
}
