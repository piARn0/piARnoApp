//
// Created by JW on 25/06/2022.
//

#include "Engine.h"
#include <vector>
#include <openxr/openxr.h>
#include <string>

Scene* global::scene = nullptr;
Engine* global::engine = nullptr;
Piarno* global::piarno = nullptr;

void log(std::string s) {
    LOGE("%s", s.c_str());
}


Engine::Engine(Scene *scene) : scene(scene) {
    global::scene = scene;
    global::engine = this;
    global::piarno = &piarno;

#define register_io(button) buttonStates[(size_t) IO::button] = &scene-> button##Pressed;
    register_io(leftTrigger);
    register_io(rightTrigger);
    register_io(leftSqueeze);
    register_io(rightSqueeze);
    register_io(xButton);
    register_io(yButton);
    register_io(aButton);
    register_io(bButton);

    for(int i = 0; i < 2; i++) {
        auto &c = scene->trackedController[i*2];
        Rigid r{getGeometry(Mesh::cube)};
        r.pos = c.pose.Translation;
        r.rot = vec3{c.pose.Rotation.x, c.pose.Rotation.y, c.pose.Rotation.z};
        r.scl = vec3{0.01f, 0.01f, 0.01f};
        r.col = color{255, 255, 255, 255};
        r.radius = 0.03;
        controllers.push_back(std::move(r));
    }

    piarno.init();
}

uint64_t Engine::getFrame() {
    return frame;
}

const std::vector<Rigid>& Engine::getControllers() {
    return controllers;
}

bool Engine::isButtonPressed(IO button) {
    return *buttonStates[(size_t) button] == XR_TRUE;
}

float Engine::getRightTriggerHoldLevel() {
    return scene->rightTriggerHoldLevel;
}

Geometry *Engine::getGeometry(Mesh mesh) {
    return &scene->geometries[(size_t) mesh];
}

float Engine::textWidth(const std::string &text) {
    float xOff = 0;
    for (const auto &c: text) {
        if (isspace(c))
            xOff += fontWidth[0];
        else if (auto alpha = toupper(c) - 'A'; 0 <= alpha && alpha < 26)
            xOff += fontWidth[alpha] + 0.1;
    }
    return xOff - (text.size() == 0 ? 0 : 0.1);
}

void Engine::renderText(const std::string &text, vec3 pos, vec3 scl, vec3 rot, color col, bool centered) {
    scene->geometries[0].updateColors(std::vector<color_t>{col.r, col.g, col.b, col.a});

    float xOff = centered ? -textWidth(text)/2 : 0;
    float yOff = centered ? -0.4 : 0;
    for (const auto &c: text) {
        if (isspace(c)) {
            xOff += fontWidth[0];
            continue;
        }
        auto alpha = toupper(c) - 'A';
        if (0 <= alpha && alpha < 26) {
            mat4 trans = translate(pos) * rotate(rot) * scale(scl) * translate(vec3{xOff, yOff, 0});

            scene->geometries[alpha].render(trans);
            xOff += fontWidth[alpha] + 0.1;
        }
    }
}

void Engine::update() {
    frame++;

    for(int i=0; i<controllers.size(); i++) {
        auto &c = scene->trackedController[i*2];
        if(c.active) {
            auto &r = controllers[i];
            r.pos = c.pose.Translation;
            r.rot = vec3{c.pose.Rotation.x, c.pose.Rotation.y, c.pose.Rotation.z};
        }
    }

    piarno.update();
}

void Engine::render() {
    piarno.render();

    //render controllers
    for(auto &c : controllers)
        c.render();


    //DEBUG render all loaded meshes
    /*float x = -1, y = 0, z = -1;
    getGeometry(Mesh::axes)->render(mat4::Translation(x, y, z));
    int i = 0;
    for (auto &g: scene->geometries) {
        if (i > 25) {
            g.render(mat4::Translation(x, y, z));
            x += i > 25 ? 1 : fontWidth[i] + 0.01;
        }
        i++;
    }*/
}

std::array<float, 26> Engine::fontWidth;


std::vector <Geometry> Engine::loadGeometries() {
    std::vector <Geometry> g((size_t) Mesh::NUM);

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
        std::array<std::vector<vertex_t>, 26> allVertices;
        std::vector <uint8_t> vertexAlphabetLookup(vertices.size() / 3);
        std::array<std::vector<index_t>, 26> allIndices;
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
            vec3 min{std::numeric_limits<float>::max()}, max{std::numeric_limits<float>::lowest()};

            for (size_t j = 0; j < allVertices[i].size(); j += 3) {
                vec3 v{allVertices[i][j], allVertices[i][j+1], allVertices[i][j+2]};
                min = vec3::Min(min, v);
                max = vec3::Max(max, v);
            }
            //align to lower left corner
            for (size_t j = 0; j < allVertices[i].size(); j += 3) {
                allVertices[i][j + 0] -= min.x;
                allVertices[i][j + 1] -= min.y;
                allVertices[i][j + 2] -= min.z;
            }
            //scale to 1m height
            for (auto &v: allVertices[i])
                v *= scale;

            fontWidth[i] = (max.x - min.x) * scale;
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
            g[i] = Geometry(allVertices[i], allIndices[i]);
    }

    {
#include "models/axes.h"

        g[(size_t) Mesh::axes] = Geometry(vertices, colors, indices, GL_LINES);
    }

    {
#include "models/cube.h"

        g[(size_t) Mesh::cube] = Geometry(vertices, indices);
    }

    {
#include "models/rect.h"

        g[(size_t) Mesh::rect] = Geometry(vertices, indices);
    }

    {
#include "models/piano_wireframe.h"

        g[(size_t) Mesh::wireframe] = Geometry(vertices, indices, GL_LINES);
    }

    {
#include "models/teapot.h"

        for (auto &v: vertices)
            v /= 100.0;

        g[(size_t) Mesh::teapot] = Geometry(vertices, indices);
    }

    return g;
}


