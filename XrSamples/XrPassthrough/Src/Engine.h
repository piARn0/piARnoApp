//
// Created by JW on 25/06/2022.
//

#pragma once

#include "XrPassthroughGl.h"

//represents a single stateful rectangle that can be rendered
class Rectangle {
public:
    Rectangle();
    void render();

    static Geometry *geometry;

//private:
    unsigned char r, g, b, a;
    float posX, posY, posZ;
    float rotX, rotY, rotZ;
    float sclX = 1, sclY = 1, sclZ = 1;
};


//a layer that abstracts away a lot of the confusing OpenXR implementation details
class Engine {
public:
    //getInputState()
    OVR::Matrix4f getControllerPose(int index);
    //etc...


};

