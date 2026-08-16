class Vector2D {
public:
    int x;
    int y;

    int getX() {
        return this->x;
    }

    int getY() {
        return this->y;
    }

    int dot(Vector2D* other) {
        return (this->x * other->x) + (this->y * other->y);
    }
};

int main() {
    Vector2D v1;
    v1.x = 3;
    v1.y = 4;

    Vector2D v2;
    v2.x = 5;
    v2.y = 6;

    int dotProduct = v1.dot(&v2); // (3*5) + (4*6) = 15 + 24 = 39
    return dotProduct;
}
