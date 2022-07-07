//
// Created by JW on 24/06/2022.
//

#include <sstream>

#include "Piarno.h"
#include "Engine.h"
#include "XrPassthroughGl.h"


bool isBlack(int index) {
    static const bool blackIndex[12] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0};

    return blackIndex[(index + 12 - 3) % 12];
}


void Piarno::init(Engine *e) {
    engine = e;

    float x = 0;
    float widthWhite = 0.02215, widthBlack = 0.011;
    float gap = 0.0005; //gap between keys
    for (size_t i = 0; i < pianoKeys.size(); i++) {
        auto &k = pianoKeys[i];

        k.geometry = engine->getGeometry(Mesh::rect);

        k.rotX = M_PI / 2;
        k.rotY = 0;
        k.rotZ = 0;

        k.a = 200;

        if (!isBlack(i)) //white key
        {
            k.posX = x;
            k.posY = -1;
            k.posZ = -1;

            k.sclX = widthWhite - gap;
            k.sclY = 0.126;
            k.sclZ = 1;

            k.r = k.g = k.b = 255;

            x += widthWhite;
        } else //black key
        {
            k.posX = x - widthWhite / 2;
            k.posY = -1 + 0.005; //float above
            k.posZ = -1 - 0.024;

            k.sclX = widthBlack - gap;
            k.sclY = 0.08;
            k.sclZ = 1;

            k.r = k.g = k.b = 0;
        }
    }

#include "songs/supermario.h"

    std::stringstream file(std::string(bytes, bytes + sizeof(bytes)));
    midi = smf::MidiFile(file);
    midi.absoluteTicks();
    midi.joinTracks();
    log("!!!!!!!!!!!!numEvents = " + std::to_string(midi.getNumEvents(0)) + "\n");


    for (int i = 0; i < midi.getNumEvents(0); i++) {
        log("tick=" + std::to_string(midi[0][i].tick));
        log("command=" + std::to_string(midi[0][i][0]));
        log("key=" + std::to_string(midi[0][i][1]));
    }

    pauseButton.geometry = engine->getGeometry(Mesh::teapot);
    pauseButton.posX = -0.2;
    pauseButton.posY = -1;
    pauseButton.posZ = -1;
    pauseButton.r = pauseButton.b = pauseButton.g = pauseButton.a = 255;
    pauseButton.sclX = pauseButton.sclY = pauseButton.sclZ = 0.1;
}

void Piarno::update() {
    if(!isPaused)
        currentTick++;
    // TODO: use controller to define this pos
//    auto ctrl_l = engine->getControllerPose(0).Translation;
//    auto ctrl_r = engine->getControllerPose(2).Translation;

    // make piano surface flat
//    piano_surface.rotX = M_PI / 2;
//    piano_surface.rotZ = 0;
//
//    piano_surface.sclX = 1.0; //width in meters
//    piano_surface.sclY = 0.126;
//    piano_surface.sclZ = 1.0; //height of key in meters

    //TODO: make relative to stage pos (floor)
//    if (engine->isButtonPressed(IO::rightTrigger) && engine->isButtonPressed(IO::leftTrigger)) {
//        piano_surface.posX = (ctrl_l.x + ctrl_r.x) / 2;
//        piano_surface.posY = (ctrl_l.y + ctrl_r.y) / 2;
//        piano_surface.posZ = (ctrl_l.z + ctrl_r.z) / 2;
//
//        // make it follow one controller
//        piano_surface.rotY = atan2(ctrl_r.x - ctrl_l.x, ctrl_r.z - ctrl_l.z) + M_PI / 2;
//
//        piano_surface.r = piano_surface.g = piano_surface.b = piano_surface.a = 255;
//    }


    int beginTick = 72 * 2;
    for(int i = 0; i < midi.getNumEvents(0); i++) {
        int command = midi[0][i][0];
        int key = midi[0][i][1];
        //int vel = midi[0][i][2];

        if(currentTick - beginTick == midi[0][i].tick / 10)
        {
            if(command == 0x90) //key press
            {
                auto &k = pianoKeys[key];
                k.r = 255;
                k.g = 0;
                k.b = 0;
            }
            if(command == 0x80) //key release
            {
                auto &k = pianoKeys[key];
                if(!isBlack(key)) {
                    k.r = k.g = k.b = 255;
                }
                else {
                    k.r = k.g = k.b = 0;
                }
            }
        }
    }
}

void Piarno::render() {
    engine->renderText("WELCOME TO PIARNO", -1, 1, -2, 0.5, 0.5, 0.1, 0, 0, 0, 255, 255, 255, 255);

    pauseButton.render();

    for (auto &k: pianoKeys)
        k.render();
}
