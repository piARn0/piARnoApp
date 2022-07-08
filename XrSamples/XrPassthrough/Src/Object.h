#pragma once

#include "XrPassthroughGl.h"

struct Geometry;

/// USEFUL TYPES AND FUNCTIONS
using vec3 = OVR::Vector3f;
using mat4 = OVR::Matrix4f;

struct color {
    color_t r, g, b, a;
};

mat4 translate(vec3 rot);
mat4 scale(vec3 rot);
mat4 rotate(vec3 rot);


/// OBJECTS

//represents a single stateful rectangle that can be rendered
class Object {
public:
    Object(Geometry *geometry = nullptr);
    void render();
    void render(const mat4 &transformation);

    vec3 pos{0, 0, 0};
    vec3 rot{0, 0, 0};
    vec3 scl{1, 1, 1};
    color col{255, 255, 255, 255};

    Geometry *geometry;
};

//represents a group of objects that are stuck together (their positions and rotations are local)
class ObjectGroup {
public:
    ObjectGroup();
    void render();

    Object& operator[](size_t i);

    vec3 pos{0, 0, 0};
    vec3 rot{0, 0, 0};
    vec3 scl{1, 1, 1};

    std::vector<Object> objects;
};

//represents an object with a cubic bounding box that can detect collision with another rigid
class Rigid : public Object {
public:
    Rigid(Geometry *geometry = nullptr);

    bool doesCollide(const Rigid &other);

    //offset position (center of bounding box) relative to Object.pos
    vec3 offset{0, 0, 0};
    //size of bounding box in meters
    vec3 size{1, 1, 1};
};

//represents a push button UI element
class Button : public Rigid {
public:
    Button(Geometry *geometry = nullptr);
};