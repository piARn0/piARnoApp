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
    //internal helpers
    void buildPiano(int numKeys);
    void loadMidi();

    Engine *engine;

    bool pauseAlreadyChanged = false;
    bool isPaused = true;
    Object pauseButton;
    std::vector<Object> pianoKeys;
    ObjectGroup pianoScene;
    smf::MidiFile midi;
    int currentTick = 0;
};

