//
// Created by JW on 25/06/2022.
//

#pragma once

#include <openxr/openxr.h>
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

namespace Side {
    const int LEFT = 0;
    const int RIGHT = 1;
    const int COUNT = 2;
}  // namespace Side


//a layer that abstracts away a lot of the confusing OpenXR implementation details
//use this class to communicate with the rest of the openxr stuffs
class Engine {
    struct InputState {
        XrActionSet actionSet{XR_NULL_HANDLE};
        XrAction grabAction{XR_NULL_HANDLE};
        XrAction poseAction{XR_NULL_HANDLE};
        XrAction vibrateAction{XR_NULL_HANDLE};
        XrAction quitAction{XR_NULL_HANDLE};
        std::array<XrPath, Side::COUNT> handSubactionPath;
        std::array<XrSpace, Side::COUNT> handSpace;
        std::array<float, Side::COUNT> handScale = {{1.0f, 1.0f}};
        std::array<XrBool32, Side::COUNT> handActive;
    };

public:
    Engine(Scene *scene);
    //getInputState()
    OVR::Posef getControllerPose(int index);

    Geometry* getGeometry(Mesh mesh);
    //etc...


    //API calls for lower level stuff (OpenXR and OpenGL)
    void update();
    void render();
    static std::vector<Geometry> load_geometries();
    void initializeActions();

protected:
    Scene *scene;
    Piarno piarno;

    XrInstance m_instance{XR_NULL_HANDLE};
    XrSession m_session{XR_NULL_HANDLE};
    XrSystemId m_systemId{XR_NULL_SYSTEM_ID};
    std::vector<XrView> m_views;
    InputState m_input;
};
