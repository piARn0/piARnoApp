//
// Created by JW on 24/06/2022.
//

#include "piarno.h"
#include "XrPassthroughGl.h"

void piarno::init(Scene *sc) {
    scene = sc;
}

void piarno::update() {
    frame++;

    auto t = scene->trackedController[0].pose.Translation;
    rect.posX = t.x;
    rect.posY = t.y;
    rect.posZ = t.z;

    scene->trackedController[0].pose.Rotation.GetYawPitchRoll(&rect.rotY, &rect.rotX, &rect.rotZ);

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

void piarno::render() {
    r2.render();
    rect.render();


    float x = -2, y = -1, z = 1;
    for (auto &g: scene->geometries) {
        g.render(OVR::Matrix4f::Translation(x, y, z) * OVR::Matrix4f::Scaling(0.1));
        x += 1;
    }
}
