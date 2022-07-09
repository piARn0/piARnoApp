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

void Piarno::buildPiano(int numKeys) {
    pianoKeys.resize(numKeys);

    float x = 0;
    float widthWhite = 0.0236, widthBlack = 0.011;
    float gap = 0.0005; //gap between keys

    for (int i = 0; i < numKeys; i++) {
        auto &k = pianoKeys[i];
        k.geometry = engine->getGeometry(Mesh::rect);

        k.rot = vec3{M_PI / 2, 0, 0};

        if (!isBlack(i)) //white key
        {
            k.pos = vec3{x, 0, 0};
            k.scl = vec3{widthWhite - gap, 0.126, 1};
            k.col = color{255, 255, 255, 150};

            x += widthWhite;
        } else //black key
        {
            k.pos = vec3{x - widthWhite / 2, 0.005, -0.024};
            k.scl = vec3{widthBlack - gap, 0.08, 1};
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

void Piarno::init(Engine *e) {
    engine = e;

    buildPiano(88);

    pauseButton.geometry = engine->getGeometry(Mesh::teapot);
    pauseButton.pos = pianoKeys[0].pos;
    pauseButton.pos.x -= 0.1;
    pauseButton.scl = vec3{0.1, 0.1, 0.1};
    pauseButton.col = color{255, 255, 255, 255};
    pauseButton.radius = 0.05;
    pianoScene.attach(pauseButton);

    loadMidi();
}

void Piarno::update() {
    if(!isPaused)
        currentTick++;

    pauseButton.update(engine->getControllers());

    //Process Midi events
    int beginTick = 72 * 2;
    for(int i = 0; i < midi.getNumEvents(0); i++) {
        int command = midi[0][i][0];
        int key = midi[0][i][1] + 3 - 12;
        //int vel = midi[0][i][2];

        if((currentTick - beginTick) == midi[0][i].tick / 2)
        {
            if(command == 0x90) //key press
            {
                auto &k = pianoKeys[key];
                k.col = color{255, 0, 0, k.col.a};
            }
            if(command == 0x80) //key release
            {
                auto &k = pianoKeys[key];
                if(!isBlack(key)) {
                    k.col = color{255, 255, 255, k.col.a};
                }
                else {
                    k.col = color{0, 0, 0, k.col.a};
                }
            }
        }
    }

    //Follow controller
    auto ctrl_l = engine->getControllers()[0].pos;
    auto ctrl_r = engine->getControllers()[2].pos;
    if (engine->isButtonPressed(IO::rightTrigger) && engine->isButtonPressed(IO::leftTrigger)) {
        pianoScene.pos = (ctrl_l + ctrl_r) / 2;
        pianoScene.pos.y -= 0.1;
        pianoScene.rot.y = atan2(ctrl_r.x - ctrl_l.x, ctrl_r.z - ctrl_l.z) - M_PI/2;
    }


    // if the joystick is in the proximity of the pauseButton, flip the `isPaused` state (=press the button)
    // and lock it until the joystick leaves that area far enough to be allowed to press the button once again
    // this approach prevents continuously flipping the `isPaused` when the joystick remains in the vicinity of the button
    if (pauseButton.isPressed())
        isPaused = !isPaused;

    // make the pauseButton either red or green displaying the current `isPaused` state
    if (isPaused) {
        pauseButton.col = color{255, 0, 0, pauseButton.col.a};
    } else {
        pauseButton.col = color{0, 255, 0, pauseButton.col.a};
    }
//
//    //debug wavy piano
//    for(auto &k : pianoKeys) {
//        k.pos.y = sin(k.pos.x*10.0f + currentTick / 10.f) * 0.005;
//    }
}

void Piarno::render() {
    engine->renderText("WELCOME TO PIARNO",
                       vec3{-1, 1, -2},
                       vec3{0.5, 0.5, 0.1},
                       vec3{0, 0, 0},
                       color{255, 255, 255, 255});

    //NON-TRANSLUCENT OBJECTS:
    //pauseButton.render();

    //TRANSLUCENT OBJECTS:
    pianoScene.render();
}



void Piarno::loadMidi() {
    //load midi file
    {
#include "songs/elise.h"

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
    }
}
