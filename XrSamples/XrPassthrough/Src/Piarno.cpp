//
// Created by JW on 24/06/2022.
//

#include "Piarno.h"
#include "Engine.h"
#include "XrPassthroughGl.h"

#include <android/log.h>

#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, OVR_LOG_TAG, __VA_ARGS__)
#define ALOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, OVR_LOG_TAG, __VA_ARGS__)

#define OVR_LOG_TAG "PIARNO"

void Piarno::init(Engine *e) {
    engine = e;

    piano_surface.geometry = engine->getGeometry(Mesh::rect);
    notes_background.geometry = engine->getGeometry(Mesh::rect);

    piano_surface.r = piano_surface.g = piano_surface.b = piano_surface.a = 150;
}

void Piarno::update() {
    frame++; //TODO: move this to Engine::update()

    //TODO: use controller to define this pos
    auto ctrl_l = engine->getControllerPose(0).Translation;
    auto ctrl_r = engine->getControllerPose(2).Translation;
    piano_surface.posX = (ctrl_l.x + ctrl_r.x) / 2;
    piano_surface.posY = (ctrl_l.y + ctrl_r.y) / 2; //TODO: make relative to stage pos (floor)
    piano_surface.posZ = (ctrl_l.z + ctrl_r.z) / 2;

    // make piano surface flat
    piano_surface.rotX = M_PI / 2;
    piano_surface.rotZ = 0;
    
    // make it follow one controller
    piano_surface.rotY = atan2(ctrl_r.x - ctrl_l.x, ctrl_r.z - ctrl_l.z) + M_PI / 2;

    piano_surface.sclX = 1.0; //width in meters
    piano_surface.sclY = 0.126;
    piano_surface.sclZ = 1.0; //height of key in meters

    if (engine->getButtonState(IO::leftSqueeze))
        ALOGE("LEFT SQUEEZE\n");
    if (engine->getButtonState(IO::xButton))
        ALOGE("X BUTTON\n");
}

void Piarno::render() {
    //notes_background.render();
    piano_surface.render();
}
