//
// Created by JW on 24/06/2022.
//
#pragma once

#include "Object.h"
#include "midi/MidiFile.h"

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

    bool pauseAlreadyChanged = false;
    bool isPaused = true; //TODO: default to true
    Object pauseButton;

    std::vector<Object> pianoKeys{88};
    smf::MidiFile midi;
    int currentTick = 0;
};

