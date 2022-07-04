//
// Created by JW on 24/06/2022.
//

#include "Piarno.h"
#include "Engine.h"
#include "XrPassthroughGl.h"

void Piarno::init(Engine *e) {
    engine = e;

    rect.geometry = engine->getGeometry(Mesh::rect);
    r2.geometry = engine->getGeometry(Mesh::rect);
}

void Piarno::update() {
    frame++; //TODO: move this to Engine::update()

    auto t = engine->getControllerPose(0).Translation;
    rect.posX = t.x;
    rect.posY = t.y;
    rect.posZ = t.z;

    engine->getControllerPose(0).Rotation.GetYawPitchRoll(&rect.rotY, &rect.rotX, &rect.rotZ);

    rect.sclX = rect.sclY = rect.sclZ = 0.3 + sin(frame / 22.0) * 0.2;

    rect.r = (sin(frame / 22.0) + 1.0) / 2 * 255;
    rect.g = rect.b = 100;
    rect.a = 200;
    //rect.r = rect.g = rect.b = rect.a = 255;

    r2.b = 255;
    r2.a = 255;
    r2.sclX = r2.sclY = r2.sclZ = 0.1;
    r2.posX = r2.posY = r2.posZ = 0;
    r2.rotX += 0.01;
    r2.rotY += 0.01;
    r2.rotZ += 0.01;
}

void Piarno::render() {
    r2.render();
    rect.render();
}