//
// Created by JW on 24/06/2022.
//
#pragma once

#include "Engine.h"


class piarno {
public:
    void init(Scene *scene);

    //run once per frame after input/state update and before rendering
    void update();

    //run once per frame to render
    void render();

private:
    int frame = 0;
    Rectangle rect, r2;

    Scene *scene;
};

