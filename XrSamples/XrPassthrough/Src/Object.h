#pragma once

struct Geometry;

//represents a single stateful rectangle that can be rendered
class Object {
public:
    Object(Geometry *geometry = nullptr);
    void render();

//private:
    float posX, posY, posZ;
    float rotX, rotY, rotZ;
    float sclX = 1, sclY = 1, sclZ = 1;
    unsigned char r, g, b, a;

    Geometry *geometry;
};