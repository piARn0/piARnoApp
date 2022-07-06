//
// Created by JW on 25/06/2022.
//

#include "Engine.h"
#include <vector>
#include <openxr/openxr.h>

using namespace OVR;


Engine::Engine(Scene *scene) : scene(scene) {
#define register_io(button) buttonStates[(size_t) IO::button] = &scene-> button##Pressed;
    register_io(leftTrigger);
    register_io(rightTrigger);
    register_io(leftSqueeze);
    register_io(rightSqueeze);
    register_io(xButton);
    register_io(yButton);
    register_io(aButton);
    register_io(bButton);

    piarno.init(this);
}

OVR::Posef Engine::getControllerPose(int index) {
    return scene->trackedController[index].pose;
}

Geometry* Engine::getGeometry(Mesh mesh) {
    return &scene->geometries[(size_t) mesh];
}

bool Engine::isButtonPressed(IO button) {
    return *buttonStates[(size_t) button] == XR_TRUE;
}

float Engine::getRightTriggerHoldLevel() {
    return scene->rightTriggerHoldLevel;
}

void Engine::update() {
    //TODO: update controller pos, update system timer, etc...
    piarno.update();
}

void Engine::render() {
    piarno.render();

    float x = -2, y = 0, z = -1;
    for (auto &g: scene->geometries) {
        g.render(OVR::Matrix4f::Translation(x, y, z) * OVR::Matrix4f::Scaling(0.2));
        x += 1;
    }
}

std::vector<Geometry> Engine::loadGeometries() {
    std::vector<Geometry> g;

    //TODO: RN THIS REQUIRES U TO LOAD AND PUSH OBJS IN THE EXACT SAME ORDER AS DEFINED IN THE ENUM
    {
#include "models/axes.h"
        g.emplace_back(std::move(vertices), std::move(colors), std::move(indices), GL_LINES);
    }

    {
#include "models/cube.h"
        g.emplace_back(std::move(vertices), std::move(colors), std::move(indices));
    }

    {
#include "models/rect.h"
        g.emplace_back(std::move(vertices), std::move(colors), std::move(indices));
    }

//    {
//#include "models/line.h"
//        g.push_back(Geometry(std::move(vertices), std::move(colors), std::move(indices)));
//    }

    {
        #include "models/teapot.h"

        for(auto &v : vertices)
            v /= 100.0;
        std::vector<unsigned char> colors(vertices.size() * 4 / 3, 255);

        g.emplace_back(std::move(vertices), std::move(colors), std::move(indices));
    }
    {
        #include "models/alphabet.h"
        std::vector<unsigned char> colors(vertices.size() * 4 / 3, 255);

        g.emplace_back(std::move(vertices), std::move(colors), std::move(indices));
    }

    return g;
}