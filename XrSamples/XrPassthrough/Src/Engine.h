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

enum class IO : size_t {
    leftTrigger,
    rightTrigger,
    leftSqueeze,
    rightSqueeze,
    xButton,
    yButton,
    aButton,
    bButton,
    NUM
};

//a layer that abstracts away a lot of the confusing OpenXR implementation details
//use this class to communicate with the rest of the openxr stuffs
class Engine {

public:
    /// API calls for higher level stuff (piARno)

    //input

    OVR::Posef getControllerPose(int index);

    // Button presses getters
    bool isButtonPressed(IO button);

    // Button holding getters
    float getRightTriggerHoldLevel();

    Geometry* getGeometry(Mesh mesh);

    /**************** YOU ARE NOW ENTERING LOW LEVEL ****************/

    //API calls for lower level stuff (OpenXR and OpenGL)
    Engine(Scene *scene);
    void update();
    void render();
    static std::vector<Geometry> loadGeometries();


protected:
    Scene *scene;
    Piarno piarno;

    std::array<XrBool32*, (size_t) IO::NUM> buttonStates;
};
