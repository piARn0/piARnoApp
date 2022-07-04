//
// Created by JW on 25/06/2022.
//

#include "Engine.h"
#include <vector>

using namespace OVR;


Engine::Engine(Scene *scene) : scene(scene) {
    piarno.init(this);
}

OVR::Posef Engine::getControllerPose(int index) {
    return scene->trackedController[index].pose;
}

Geometry* Engine::getGeometry(Mesh mesh) {
    return &scene->geometries[static_cast<size_t>(mesh)];
}

void Engine::update() {
    //TODO: update controller pos, update system timer, etc...
    piarno.update();
}

void Engine::render() {
    piarno.render();

    float x = -2, y = 0, z = -1;
    for (auto &g: scene->geometries) {
        g.render(OVR::Matrix4f::Translation(x, y, z) * OVR::Matrix4f::Scaling(0.1));
        x += 1;
    }
}

std::vector<Geometry> Engine::load_geometries() {
    std::vector<Geometry> g;

    //TODO: RN THIS REQUIRES U TO LOAD AND PUSH OBJS IN THE EXACT SAME ORDER AS DEFINED IN THE ENUM
    {
#include "models/axes.h"
        g.emplace_back(std::move(vertices), std::move(colors), std::move(indices));
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

