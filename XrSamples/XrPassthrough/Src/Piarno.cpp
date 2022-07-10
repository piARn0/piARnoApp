//
// Created by JW on 24/06/2022.
//

#include <sstream>

#include "Piarno.h"
#include "Engine.h"
#include "XrPassthroughGl.h"

using namespace global;

void Piarno::init() {
    buildPiano();

    pauseButton.geometry = engine->getGeometry(Mesh::cube);
    pauseButton.pos = pianoKeys[2].pos;
    pauseButton.pos.z += 0.2;
    pauseButton.scl = vec3{0.03, 0.02, 0.03};
    pauseButton.col = color{255, 255, 255, 255};
    pauseButton.radius = 0.02;
    pauseButton.label = "PLAY";
    pianoScene.attach(pauseButton);

    slider.geometry = engine->getGeometry(Mesh::cube);
    slider.pos = pauseButton.pos;
    slider.pos.x += 0.1;
    slider.scl = vec3{0.03, 0.02, 0.03};
    slider.col = color{255, 255, 0, 255};
    slider.radius = 0.02;
    slider.label = "TIME"; //TODO: show current time in minutes:seconds instead
    slider.min = 0;
    slider.max = 0.5;
    pianoScene.attach(slider);

    loadMidi();

    createTiles();
}


void Piarno::update() {
    if(!isPaused) {
        currentTime += 1.0 / 72.0 * speedMultiplier;
    }

    //buttons
    const auto &controllers = engine->getControllers();
    pauseButton.update(controllers);
    slider.update(controllers);

    if (pauseButton.isPressed())
        isPaused = !isPaused;

    if (slider.isBeingPressed()) {
        slider.col = color{200, 200, 0, 255};
        isPaused = true;
        currentTime = slider.getVal() / slider.max * midi.getFileDurationInSeconds();
        currentEvent = 0; //search from beginning
        //reset all highlighted colors TODO: put this into a neat function...
        for(size_t i = 0; i < numKeys; i++) {
            auto &k = pianoKeys[i];
            if (!isBlack(i)) {
                k.col = color{255, 255, 255, k.col.a};
            } else {
                k.col = color{0, 0, 0, k.col.a};
            }
        }
    }
    else {
        slider.col = color{255, 255, 0, 255};
        slider.setVal(currentTime / midi.getFileDurationInSeconds() * slider.max);
    }

    // make the pauseButton either red or green displaying the current `isPaused` state
    if (isPaused) {
        pauseButton.col = color{255, 0, 0, pauseButton.col.a};
        pauseButton.label = "PLAY";
    } else {
        pauseButton.col = color{0, 255, 0, pauseButton.col.a};
        pauseButton.label = "PAUSE";
    }

    //set piano position with controller
    auto ctrlL = controllers[0].pos;
    auto ctrlR = controllers[2].pos;
    if (engine->isButtonPressed(IO::rightTrigger) && engine->isButtonPressed(IO::leftTrigger)) {
        pianoScene.pos = (ctrlL + ctrlR) / 2;
        pianoScene.pos.y -= 0.1;
        pianoScene.rot.y = atan2(ctrlR.x - ctrlL.x, ctrlR.z - ctrlL.z) - M_PI/2;
    }


    //Process Midi events
    //TODO: optimize this (using Tiles) and pack it away
    for(; currentEvent < midi.getNumEvents(0); currentEvent++) {
        int i = currentEvent;
        int command = midi[0][i][0];
        int key = midi[0][i][1] - offset;
        //int vel = midi[0][i][2];

        if(currentTime >= midi[0][i].seconds)
        {
            if(0 <= key && key < numKeys) {
                if (command == 0x90) //key press
                {
                    auto &k = pianoKeys[key];
                    k.col = color{255, 0, 0, k.col.a};
                }
                if (command == 0x80) //key release
                {
                    auto &k = pianoKeys[key];
                    if (!isBlack(key)) {
                        k.col = color{255, 255, 255, k.col.a};
                    } else {
                        k.col = color{0, 0, 0, k.col.a};
                    }
                }
            }
        }
        else //no more events to process rn...
            break;
    }

    updateTiles();
}


void Piarno::render() {
    engine->renderText("WELCOME TO PIARNO",
                       vec3{0, 1 + sin(engine->getFrame() / 72.0f) * 0.05f, -2},
                       vec3{0.5, 0.5, 0.3},
                       vec3{0, 0, 0},
                       color{255, 255, 255, 255});

    //NON-TRANSLUCENT OBJECTS:
    //pauseButton.render(); //is now part of pianoScene

    //TRANSLUCENT OBJECTS:
    pianoScene.render();
}



bool Piarno::isBlack(int index) {
    static const bool blackIndex[12] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0};

    return blackIndex[(index + offset) % 12];
}

void Piarno::buildPiano() {
    pianoKeys.resize(numKeys);

    float x = 0;

    for (int i = 0; i < numKeys; i++) {
        auto &k = pianoKeys[i];
        k.geometry = engine->getGeometry(Mesh::rect);

        k.rot = vec3{M_PI / 2, 0, 0};

        if (!isBlack(i)) //white key
        {
            k.pos = vec3{x, 0, 0};
            k.scl = vec3{widthWhite - gap, heightWhite, 1};
            k.col = color{255, 255, 255, 150};

            x += widthWhite;
        } else //black key
        {
            k.pos = vec3{x - widthWhite / 2, 0.005, -0.024};
            k.scl = vec3{widthBlack - gap, heightBlack, 1};
            k.col = color{0, 0, 0, 150};
        }

        pianoScene.attach(k);
    }

    //center
    float width = x;
    for(auto &k : pianoKeys) {
        k.pos.x -= width/2;
    }
}

void Piarno::createTiles() {
    // count the total number of key presses events (=notes being played)
    int keyPressNum = 0;
    for (int i = 0; i < midi.getNumEvents(0); i++) {
        int command = midi[0][i][0];
        if (command == 0x90) {
            keyPressNum++;
        }
    }
    // TODO: cleanup after debugging
    log("Number of notes in this song: " + std::to_string(keyPressNum));
    allTiles.resize(keyPressNum);

    std::vector<Tile*> currentTile(numKeys, nullptr); //"currently" (within the below loop) active tile

    int k = 0; //current key press
    for (int i = 0; i < midi.getNumEvents(0); i++) {
        int command = midi[0][i][0];
        int key = midi[0][i][1] - offset;

        //FIXME: check key boundary (some songs don't fit on small pianos)
        if (command == 0x90) {
            // key press
            allTiles[k].startTime = midi[0][i].seconds;
            currentTile[key] = &allTiles[k]; //register the currently active tile for this lane
            k++;
        } else if (command == 0x80) {
            // key release
            currentTile[key]->endTime = midi[0][i].seconds;

            auto &tile = currentTile[key]->tile;
            tile.geometry = engine->getGeometry(Mesh::rect);
            tile.pos = pianoKeys[key].pos; //z will be set every frame based on current time
            //tile.pos.y += 0.01; //float above keys
            tile.rot = vec3{M_PI / 2, 0, 0};
            tile.scl = vec3{(isBlack(key) ? widthBlack : widthWhite) - gap, 1, 1}; //height will be updated
            tile.col = color{0, 0, 255, 255};

            pianoScene.attach(tile);

            currentTile[key] = nullptr;

        } else {
            // ignore any other event
            continue;
        }
    }
}

void Piarno::updateTiles() {
    for(auto &[tile, start, end] : allTiles) {
        float startDist = std::max(0.0f, distFromTime(start - currentTime)); //distance in meters to start pos
        float endDist = std::max(0.0f, distFromTime(end - currentTime)); //distance in meters to end pos

        tile.scl.y = endDist - startDist; //tile length
        tile.pos.z = -heightWhite / 2 - (startDist + tile.scl.y / 2); //center of tile
    }
}

float Piarno::distFromTime(double time) {
    return tileVelocity * time;
}


void Piarno::loadMidi() {
    //load midi file
    {
#include "songs/jacque.h"

        std::stringstream file(std::string(bytes, bytes + sizeof(bytes)));
        midi = smf::MidiFile(file);
        //midi.absoluteTicks();
        midi.joinTracks();
        midi.doTimeAnalysis(); //calculate seconds for each event
        log("!!!!!!!!!!!!numEvents = " + std::to_string(midi.getNumEvents(0)) + "\n");

//        for (int i = 0; i < midi.getNumEvents(0); i++) {
//            log("tick=" + std::to_string(midi[0][i].tick));
//            log("second=" + std::to_string(midi[0][i].seconds));
//            log("command=" + std::to_string(midi[0][i][0]));
//            log("key=" + std::to_string(midi[0][i][1]));
//        }
    }
}
