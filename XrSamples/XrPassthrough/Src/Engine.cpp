//
// Created by JW on 25/06/2022.
//

#include "Engine.h"
#include <vector>

Geometry *Rectangle::geometry = nullptr;

using namespace OVR;

void Rectangle::render() {
    //color per vertex (each corner)
    geometry->updateColors(std::vector<unsigned char> {
            r, g, b, a,
            r, g, b, a,
            r, g, b, a,
            r, g, b, a,
        });

    //set the transformation matrix and render
    auto trans = Matrix4f::Translation(posX, posY, posZ) *
                 Matrix4f::Scaling(sclX, sclY, sclZ) *
                 Matrix4f::RotationX(rotX) * Matrix4f::RotationY(rotY) * Matrix4f::RotationZ(rotZ);
    geometry->render(trans);
}

Rectangle::Rectangle() {
}


OVR::Matrix4f Engine::getControllerPose(int index) {
    return OVR::Matrix4f();
}
