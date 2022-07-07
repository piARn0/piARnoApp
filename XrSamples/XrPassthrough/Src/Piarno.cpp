//
// Created by JW on 24/06/2022.
//

#include "Piarno.h"
#include "Engine.h"
#include "XrPassthroughGl.h"


void Piarno::init(Engine *e) {
    engine = e;

    piano_surface.geometry = engine->getGeometry(Mesh::rect);
    notes_background.geometry = engine->getGeometry(Mesh::rect);

    piano_surface.r = piano_surface.g = piano_surface.b = piano_surface.a = 150;
    notes_background.r = notes_background.a = 255;
    notes_background.g = notes_background.b = 0;
    notes_background.posX = notes_background.posY = notes_background.posZ = 0;
}

void Piarno::update() {
    //TODO: use controller to define this pos
    auto ctrl_l = engine->getControllerPose(0).Translation;
    auto ctrl_r = engine->getControllerPose(2).Translation;
    piano_surface.posX = (ctrl_l.x + ctrl_r.x) / 2;
    piano_surface.posY = (ctrl_l.y + ctrl_r.y) / 2; //TODO: make relative to stage pos (floor)
    piano_surface.posZ = (ctrl_l.z + ctrl_r.z) / 2;

    piano_surface.rotX = M_PI / 2; //make piano surface flat
    //piano_surface.rotY = atan2(ctrl_r.x - ctrl_l.x, ctrl_r.z - ctrl_l.z); //make it go from one controller

    piano_surface.sclX = 1.0; //width in meters
    piano_surface.sclZ = 0.126; //height of key in meters

    if (engine->getButtonState(IO::leftSqueeze))
        Engine::log("LEFT SQUEEZE\n");
    if (engine->getButtonState(IO::xButton))
        Engine::log("X BUTTON\n");
}

void Piarno::render() {
    engine->renderText("WELCOME TO PIARNO", -1, 1, -2, 0.5, 0.5, 0.1, 0, 0, 0, 255, 255, 255, 255);
    //notes_background.render();
    //piano_surface.render();
}
