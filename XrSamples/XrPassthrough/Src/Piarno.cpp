//
// Created by JW on 24/06/2022.
//

#include <sstream>

#include "Piarno.h"
#include "Engine.h"
#include "XrPassthroughGl.h"

//DEBUG
#include <android/log.h>
#define OVR_LOG_TAG "XrPassthrough"

#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, OVR_LOG_TAG, __VA_ARGS__)
#define ALOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, OVR_LOG_TAG, __VA_ARGS__)



bool is_black(int index) {
    static const bool black_index[12] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0};

    return black_index[(index + 12 - 3) % 12];
}

void log(std::string s) {
    ALOGE("%s", s.c_str());
}


void Piarno::init(Engine *e) {
    engine = e;

    float x = 0;
    float width_white = 0.02215, width_black = 0.011;
    float gap = 0.0005; //gap between keys
    for (size_t i = 0; i < piano_keys.size(); i++) {
        auto &k = piano_keys[i];

        k.geometry = engine->getGeometry(Mesh::rect);

        k.rotX = M_PI / 2;
        k.rotY = 0;
        k.rotZ = 0;

        k.a = 200;

        if(!is_black(i)) //white key
        {
            k.posX = x;
            k.posY = -1;
            k.posZ = -1;

            k.sclX = width_white - gap;
            k.sclY = 1;
            k.sclZ = 0.126;

            k.r = k.g = k.b = 255;

            x += width_white;
        }
        else //black key
        {
            k.posX = x - width_white / 2;
            k.posY = -1 + 0.005; //float above
            k.posZ = -1 - 0.024;

            k.sclX = width_black - gap;
            k.sclY = 1;
            k.sclZ = 0.08;

            k.r = k.g = k.b = 0;
        }
    }

    //load midi file
    //log()

#include "songs/supermario.h"
    std::stringstream file(std::string(bytes, bytes + sizeof(bytes)));
    log(file.str());
    ALOGE("!!!!!!!!parsing MIDI file...\n");
    midi = smf::MidiFile(file);
    midi.absoluteTicks();
    midi.joinTracks();
    log("!!!!!!!!!!done:\n");
    log("!!!!!!!!!!!!numEvents = " + std::to_string(midi.getNumEvents(0)) + "\n");


    for(int i=0; i< midi.getNumEvents(0); i++){
        log("tick="+std::to_string(midi[0][i].tick));
        log("command="+std::to_string(midi[0][i][0]));
        log("key="+std::to_string(midi[0][i][1]));
    }
}




void Piarno::update() {
    frame++; //TODO: move this to Engine::update()

    int begin_tick = 72 * 2;
    for(int i = 0; i < midi.getNumEvents(0); i++) {
        int command = midi[0][i][0];
        int key = midi[0][i][1];
        //int vel = midi[0][i][2];

        if(frame - begin_tick == midi[0][i].tick / 10)
        {
            if(command == 0x90) //key press
            {
                auto &k = piano_keys[key];
                k.r = 255;
                k.g = 0;
                k.b = 0;
            }
            if(command == 0x80) //key release
            {
                auto &k = piano_keys[key];
                if(!is_black(key)) {
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
    for (auto &k: piano_keys)
        k.render();
}
