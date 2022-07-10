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
    int key; //key index of this tile
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
    int numKeys = 49, offset = 24+12;
    //int numKeys = 88, offset = 12-3;
    float widthWhite = 0.0236 - 0.00007, widthBlack = 0.011;
    float heightWhite = 0.126 + 0.01, heightBlack = 0.08 + 0.01;
    float gap = 0.0005, blackHover = 0.007; //gap between keys and hover amount of black keys
    float overlayOpacity = 0.6;

    //song visualization
    std::vector<Tile> allTiles; //tile objects and their start time in seconds
    std::vector<float> keyHighlight; //highlight value for each key for incoming/current key
    float keyPressDepth = blackHover - 0.001;

    //playback & UI
    smf::MidiFile midi;
    double currentTime = 0;
    Slider playbackSpeed{0.25, 1, 2}; //min default max
    Slider timeline;

    bool isPaused = true;
    Button pauseButton;
    Slider scrollSpeed{0.01, 0.1, 5}; //min default max of tile velocity, meters per second

    Object pianoOutline; //to help aligning
    Button toggleOutline;
};

