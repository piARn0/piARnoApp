//
// Created by JW on 25/06/2022.
//

#pragma once

#include <array>
#include "XrPassthroughGl.h"
#include "Piarno.h"

enum class Mesh : size_t {
    axis,
    cube,
    rect,
    //line,
    teapot,
    alphabet,
    NUM
};

//a layer that abstracts away a lot of the confusing OpenXR implementation details
//use this class to communicate with the rest of the openxr stuffs
class Engine {

public:
    Engine(Scene *scene);
    OVR::Posef getControllerPose(int index);

    Geometry* getGeometry(Mesh mesh);
    //etc...

    //API calls for lower level stuff (OpenXR and OpenGL)
    void update();
    void render();
    static std::vector<Geometry> load_geometries();

    // Button presses callbacks
    bool getLeftTriggerState();
    bool getRightTriggerState();
    bool getLeftSqueezeState();
    bool getRightSqueezeState();

protected:
    Scene *scene;
    Piarno piarno;
};
