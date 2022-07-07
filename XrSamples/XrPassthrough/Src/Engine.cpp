//
// Created by JW on 25/06/2022.
//

#include "Engine.h"
#include <vector>
#include <openxr/openxr.h>
#include <string>

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

uint64_t Engine::getFrame() {
    return frame;
}

OVR::Posef Engine::getControllerPose(int index) {
    return scene->trackedController[index].pose;
}

bool Engine::getButtonState(IO button) {
    return *buttonStates[(size_t) button] == XR_TRUE;
}

float Engine::getRightTriggerHoldLevel() {
    return scene->rightTriggerHoldLevel;
}

Geometry *Engine::getGeometry(Mesh mesh) {
    return &scene->geometries[(size_t) mesh];
}

void Engine::renderText(std::string text, float x, float y, float z, float sX, float sY, float sZ,
                        float rX, float rY, float rZ, color_t r, color_t g, color_t b, color_t a) {
    //TODO: apply color
    float xOff = 0;
    for (const auto &c: text) {
        if(isspace(c)){
            xOff += fontWidth[0];
        }
        else {
            auto alpha = toupper(c) - 'A';
            if (0 <= alpha && alpha < 26) {
                Matrix4f trans = Matrix4f::Translation(x, y, z)
                                 * (Matrix4f::RotationZ(rZ) *
                                    (Matrix4f::RotationY(rY) * (Matrix4f::RotationX(rX)
                                                                * (Matrix4f::Scaling(sX, sY, sZ) *
                                                                   Matrix4f::Translation(xOff, 0,
                                                                                         0)))));
                scene->geometries[alpha].render(trans);
                xOff += fontWidth[alpha] + sX * 0.2;
            }
        }
    }
}

void Engine::log(std::string s) {
    LOGE("%s", s.c_str());
}

void Engine::update() {
    frame++;

    piarno.update();
}

void Engine::render() {
    piarno.render();

    //render controllers
    for (int i = 0; i < 4; i++) {
        if (!scene->trackedController[i].active)
            continue;
        Matrix4f pose(scene->trackedController[i].pose);
        Matrix4f scale;
        if (i & 1) {
            scale = Matrix4f::Scaling(0.03f, 0.03f, 0.03f);
        } else {
            scale = Matrix4f::Scaling(0.02f, 0.02f, 0.06f);
        }
        getGeometry(Mesh::cube)->render(pose * scale);
    }


    //DEBUG render all loaded meshes
    /*float x = -1, y = 0, z = -1;
    getGeometry(Mesh::axes)->render(Matrix4f::Translation(x, y, z));
    int i = 0;
    for (auto &g: scene->geometries) {
        if (i > 25) {
            g.render(OVR::Matrix4f::Translation(x, y, z));
            x += i > 25 ? 1 : fontWidth[i] + 0.01;
        }
        i++;
    }*/
}

std::array<float, 26> Engine::fontWidth;


std::vector<Geometry> Engine::loadGeometries() {
    std::vector<Geometry> g((size_t) Mesh::NUM);

    {
#include "models/alphabet.h"
        //split the alphabets into 26 individuals
        double yMin = vertices[1], yMax = vertices[1];
        for (size_t i = 1; i < vertices.size(); i += 3) {
            if (vertices[i] < yMin)
                yMin = vertices[i];
            if (yMax < vertices[i])
                yMax = vertices[i];
        }
        double margin = (yMax - yMin) * 0.01;
        yMin -= margin;
        yMax += margin;

        double alphabetHeight = (yMax - yMin);
        std::array<std::vector<float>, 26> allVertices;
        std::vector<uint8_t> vertexAlphabetLookup(vertices.size() / 3);
        std::array<std::vector<unsigned short>, 26> allIndices;
        std::array<int, 26> firstVertexIndexPerAlphabet;
        firstVertexIndexPerAlphabet.fill(-1);

        //determine and partition vertices according to its y
        for (size_t i = 1; i < vertices.size(); i += 3) {
            auto alpha = 25 - (size_t) floor((vertices[i] - yMin) / alphabetHeight * 26);
            allVertices[alpha].push_back(vertices[i - 1]);
            allVertices[alpha].push_back(vertices[i]);
            allVertices[alpha].push_back(vertices[i + 1]);
            vertexAlphabetLookup[i / 3] = alpha;
            if (firstVertexIndexPerAlphabet[alpha] == -1)
                firstVertexIndexPerAlphabet[alpha] = i / 3;
        }

        //align the alphabet to its lower left corner (0, 0, 0) and scale to 1m height and measure width
        float scale = 1 / (alphabetHeight / 26);
        for (size_t i = 0; i < 26; i++) {
            //measure sizes/pos
            float minX, minY, minZ;
            float maxX = std::numeric_limits<float>::lowest();
            minX = minY = minZ = std::numeric_limits<float>::max();

            for (size_t j = 0; j < allVertices[i].size(); j += 3) {
                if (allVertices[i][j + 0] < minX)
                    minX = allVertices[i][j + 0];
                if (maxX < allVertices[i][j + 0])
                    maxX = allVertices[i][j + 0];
                if (allVertices[i][j + 1] < minY)
                    minY = allVertices[i][j + 1];
                if (allVertices[i][j + 2] < minZ)
                    minZ = allVertices[i][j + 2];
            }
            //align to lower left corner
            for (size_t j = 0; j < allVertices[i].size(); j += 3) {
                allVertices[i][j + 0] -= minX;
                allVertices[i][j + 1] -= minY;
                allVertices[i][j + 2] -= minZ;
            }
            //scale to 1m height
            for (auto &v: allVertices[i])
                v *= scale;

            fontWidth[i] = (maxX - minX) * scale;
        }

        //find which indices belong to which alphabet
        for (size_t i = 0; i < indices.size(); i += 3) {
            //this assumes all 3 indices of a face belong to the same alphabet
            auto alpha = vertexAlphabetLookup[indices[i]];
            auto offset = firstVertexIndexPerAlphabet[alpha];

            allIndices[alpha].push_back(indices[i] - offset);
            allIndices[alpha].push_back(indices[i + 1] - offset);
            allIndices[alpha].push_back(indices[i + 2] - offset);
        }

        for (size_t i = 0; i < 26; i++)
            g[i] = Geometry(allVertices[i],
                            std::vector<unsigned char>(allVertices[i].size() * 4 / 3, 255),
                            allIndices[i]);
    }

    {
#include "models/axes.h"

        g[(size_t) Mesh::axes] = Geometry(std::move(vertices), std::move(colors),
                                          std::move(indices), GL_LINES);
    }

    {
#include "models/cube.h"

        g[(size_t) Mesh::cube] = Geometry(std::move(vertices), std::move(colors),
                                          std::move(indices));
    }

    {
#include "models/rect.h"

        g[(size_t) Mesh::rect] = Geometry(std::move(vertices), std::move(colors),
                                          std::move(indices));
    }

//    {
//#include "models/line.h"
//        g.push_back(Geometry(std::move(vertices), std::move(colors), std::move(indices)));
//    }

    {
#include "models/teapot.h"

        for (auto &v: vertices)
            v /= 100.0;
        std::vector<unsigned char> colors(vertices.size() * 4 / 3, 255);

        g[(size_t) Mesh::teapot] = Geometry(std::move(vertices), std::move(colors),
                                            std::move(indices));
    }

    return g;
}


