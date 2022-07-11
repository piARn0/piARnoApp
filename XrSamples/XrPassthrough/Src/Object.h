#pragma once

#include "XrPassthroughGl.h"
#include "Global.h"
#include <string>
#include <optional>

/// USEFUL TYPES AND FUNCTIONS
using vec3 = OVR::Vector3f;
using mat4 = OVR::Matrix4f;

const size_t R = 0;
const size_t G = 1;
const size_t B = 2;
const size_t A = 3;

struct color {
    color(color_t r, color_t g, color_t b, color_t a, Geometry *perVertexGeom = nullptr);

    color_t& r(int i = 0);
    color_t& g(int i = 0);
    color_t& b(int i = 0);
    color_t& a(int i = 0);
    void setAll(size_t channel, color_t val);

    std::vector<color_t> data;
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
    bool show = true;

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
    float radius = 0.02;
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

    color pressCol = color{50, 50, 50, 255};
    std::string label;

protected:
    bool pressed = false, pressedPrev = false;
    float maxPress = radius;
};

//a slider that can be moved along a set track (left and right)
class Slider : public Button {
public:
    Slider(Geometry *geometry = nullptr);
    Slider(float minValue, float value, float maxValue, float left = 0, float right = 0.2, Geometry *geometry = nullptr);

    //run this once per frame
    void update(const std::vector<Rigid> &controllers);

    float get();
    void set(float val);

    void render(mat4 *postTransform = nullptr) override;

    float min = 0, max = 0.2; //slider track towards left and right
    float minVal = 0, maxVal = 1; //range of the value for set/get

protected:
    vec3 calculateOffset(vec3 controllerPos);

    float val = 0;

    vec3 trackDir{1.0f, 0.0f, 0.0f}; //direction of the track (make sure it's normalized!)

    vec3 controllerOffset{0,0,0}; //when pressed by controller, save its position so we can move along it
};