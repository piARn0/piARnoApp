//
// Created by JW on 24/06/2022.
//
#pragma once

#include "Global.h"
#include "Object.h"
#include "midi/MidiFile.h"

class Engine;

class Piarno {
public:
    void init();

    //run once per frame after input/state update and before rendering
    void update();

    //run once per frame to render
    void render();

private:
    //internal helpers
    bool isBlack(int index);
    void buildPiano();
    void loadMidi();

    //piano overlay & tiles
    ObjectGroup pianoScene;
    std::vector<Object> pianoKeys;
    int numKeys = 49, offset = 24;
    //int numKeys = 88, offset = 12-3;

    //playback & UI
    smf::MidiFile midi;
    double currentTime = 0, speedMultiplier = 0.75;
    int currentEvent = 0;
    bool isPaused = true;
    Button pauseButton;
    Slider slider;
};

