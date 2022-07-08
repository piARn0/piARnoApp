//
// Created by JW on 25/06/2022.
//

#pragma once

#include <array>
#include <string>

#include "XrPassthroughGl.h"
#include "Piarno.h"

//DEBUG LOGGING
#include "android/log.h"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "PIARNO", __VA_ARGS__)
void log(std::string s);


enum class Mesh : size_t {
    axes = 26, //ranges 0-25 are for alphabets A-Z
    cube,
    rect,
    //line,
    teapot,
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

    // General
    uint64_t getFrame();

    // Input
    OVR::Posef getControllerPose(int index);
    bool isButtonPressed(IO button);
    float getRightTriggerHoldLevel();

    // Render related
    Geometry* getGeometry(Mesh mesh);
    void renderText(std::string text, vec3 pos, vec3 scl, vec3 rot, color col);

    /**************** YOU ARE NOW ENTERING LOW LEVEL ****************/

    //API calls for lower level stuff (OpenXR and OpenGL)
    Engine(Scene *scene);
    void update();
    void render();
    static std::vector<Geometry> loadGeometries();


protected:
    Scene *scene;
    Piarno piarno;

    uint64_t frame = 0;
    std::array<XrBool32*, (size_t) IO::NUM> buttonStates;

    static std::array<float, 26> fontWidth;
};
