#pragma once

#include "XrPassthroughGl.h"
#include "Global.h"
#include <string>
#include <optional>

/// USEFUL TYPES AND FUNCTIONS
using vec3 = OVR::Vector3f;
using mat4 = OVR::Matrix4f;

struct color {
    color_t r, g, b, a;
};

mat4 translate(vec3 pos);
mat4 scale(vec3 scl);
mat4 rotate(vec3 rot);


/// OBJECTS

//represents a single stateful rectangle that can be rendered
class Object {
public:
    Object(Geometry *geometry = nullptr);
    virtual void render(mat4 *postTransform = nullptr);

    vec3 globalPos(std::optional<vec3> p = std::nullopt) const;
    vec3 globalRot(std::optional<vec3> r = std::nullopt) const;
    vec3 globalScl(std::optional<vec3> s = std::nullopt) const;

    vec3 pos{0, 0, 0};
    vec3 rot{0, 0, 0};
    vec3 scl{1, 1, 1};
    color col{255, 255, 255, 255};

    Geometry *geometry;

protected:
    friend ObjectGroup; //allow it to access parent attribute
    ObjectGroup *parent = nullptr;
};

//represents a group of objects that are stuck together (their positions and rotations are local)
class ObjectGroup {
public:
    using iterator = std::vector<Object*>::iterator;

    ObjectGroup();
    void render();

    Object& operator[](size_t i);
    iterator begin();
    iterator end();
    size_t size();
    void attach(Object &obj);

    vec3 pos{0, 0, 0};
    vec3 rot{0, 0, 0};
    vec3 scl{1, 1, 1};

protected:
    std::vector<Object*> objects;
};

//represents an object with a spherical collision body that can detect collision with another rigid
class Rigid : public Object {
public:
    Rigid(Geometry *geometry = nullptr);

    bool isColliding(const Rigid &other);

    //offset position (center of body) relative to Object.pos
    vec3 offset{0, 0, 0};
    //radius of body in meters
    float radius = 0.01;
};

//represents a push button UI element
class Button : public Rigid {
public:
    Button(Geometry *geometry = nullptr);

    //run this once per frame
    void update(const std::vector<Rigid> &controllers);

    //returns true when button is pressed once
    bool isPressed();
    //returns true when button is released once
    bool isReleased();
    //returns current status
    bool isBeingPressed();

    void render(mat4 *postTransform = nullptr) override;

    std::string label;

protected:
    bool pressed = false, pressedPrev = false;
    float maxPress = radius * 2;

    //TODO: add cooldown timer if press gets triggered multiple times at boundary
};

//a slider that can be moved along a set track (left and right)
class Slider : public Button {
public:
    Slider(Geometry *geometry = nullptr);

    //run this once per frame
    void update(const std::vector<Rigid> &controllers);

    float getVal();
    void setVal(float val);

    void render(mat4 *postTransform = nullptr) override;

    float min = -0.25f, max = 0.25f; //slider track towards left and right
    float minVal = 0, maxVal = 1; //range of the value for set/get
    vec3 trackDir{1.0f, 0.0f, 0.0f}; //direction of the track (make sure it's normalized!)

protected:
    vec3 calculateOffset(vec3 controllerPos);

    float val = 0;
    vec3 controllerOffset{0,0,0}; //when pressed by controller, save its position so we can move along it
};