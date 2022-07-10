//
// Created by JW on 24/06/2022.
//
#pragma once

#include "Global.h"
#include "Object.h"
#include "midi/MidiFile.h"

// Represents a falling tile of a note for song visualization
struct Tile {
    Object tile;
    double startTime; //timestamp of this note start
    double endTime; //timestamp of this note end
};

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
    void createTiles();
    void updateTiles();
    float distFromTime(double time);

    //piano overlay
    ObjectGroup pianoScene;
    std::vector<Object> pianoKeys;
    int numKeys = 49, offset = 24;
    //int numKeys = 88, offset = 12-3;
    float widthWhite = 0.0236, widthBlack = 0.011;
    float heightWhite = 0.126, heightBlack = 0.08;
    float gap = 0.0005; //gap between keys

    //song visualization
    std::vector<Tile> allTiles; //tile objects and their start time in seconds
    float tileVelocity = 0.5; //determines tile pos, meters per second

    //playback & UI
    smf::MidiFile midi;
    double currentTime = 0, speedMultiplier = 1;
    int currentEvent = 0; //TODO: deprecated
    bool isPaused = true;

    Button pauseButton;
    Slider timeline;
    Slider scrollSpeed;
    Slider playbackSpeed;
};

