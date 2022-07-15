//
// Created by JW on 24/06/2022.
//
#pragma once

#include "Global.h"
#include "Object.h"
#include "midi/MidiFile.h"
#include <unordered_map>

const std::vector<std::string> songs = {
    "2AM",
    "Canon",
    "Sweden",
    "Twinkle",
    "Supermario",
    "Hit the Road Jack",
    "Gymnopedie",
    "Fuer Elise",
    "Jacque",
    "Ode to Joy",
    "Heart n Soul 1",
    "Heart n Soul 2",
    "Pirate",
    "Wet Hands",
    "Sweden 2",
    "Four Seasons Spring",
    "Four Seasons Winter",
    "Game of Thrones",
    "Minuet",
    "7th Symphony 2nd Movement",
    "Fairy Tail",
    "River Flows in You",
    "Amelie Theme",
    "In the Name of Love",
    "Imagine",
    "The Winner Takes It All",
    "Somewhere over the Rainbow",
    "Paradise",
    "Let Her Go",
    "Take On Me",
    "Coffin Dance"
};

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
    void loadMidi(int index);
    void createTiles();
    void updateTiles();
    float distFromTime(double time);

    //piano overlay
    ObjectGroup pianoScene;
    std::vector<Object> pianoKeys;
    int numKeys = 49, offset = 24+12;
    //int numKeys = 88, offset = 12-3;
    float widthWhite = 0.0236, widthBlack = 0.011;
    float heightWhite = 0.126 + 0.01, heightBlack = 0.08 + 0.01;
    float gap = 0.0005, blackHover = 0.006; //gap between keys and hover amount of black keys
    float overlayOpacity = 0.6;

    //song visualization
    std::vector<Tile> allTiles; //tile objects and their start time in seconds
    std::vector<float> keyHighlight; //highlight value for each key for incoming/current key
    std::vector<color> tileColor {
        color{0, 228, 255, 255}, //cyan - track 0 white
        color{0, 188, 215, 255}, //darker cyan - track 0 black
        color{255, 134, 0, 255}, //orange - track 1
        color{215, 94, 0, 255}, //darker orange - track 1 black
        color{140, 0, 252, 255}, //violet - track 2
        color{100, 0, 212, 255}, //darker violet - track 2 black
        color{255, 254, 55, 255}, // yellow - track 3
        color{215, 214, 15, 255} // yellow - track 3 black
    };
    std::unordered_map<int, size_t> trackToIndex;
    float keyPressDepth = blackHover - 0.001;

    //playback & UI
    smf::MidiFile midi;
    double currentTime = 0, waitTimeBegin = 3;
    Slider playbackSpeed{0.25, 1, 2}; //min default max
    Slider timeline;
    Slider songListScroll;

    bool isPaused = true;
    Button pauseButton;
    Slider scrollSpeed{0.05, 0.2, 2}; //min default max of tile velocity, meters per second

    Object pianoOutline; //to help aligning
    Button toggleOutline;
};

