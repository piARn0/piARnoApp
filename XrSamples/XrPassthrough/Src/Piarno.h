//
// Created by JW on 24/06/2022.
//
#pragma once

#include "Object.h"
class Engine;

class Piarno {
public:
    void init(Engine *engine);

    //run once per frame after input/state update and before rendering
    void update();

    //run once per frame to render
    void render();

private:
    Engine *engine;

    Object piano_surface, notes_background;
};

