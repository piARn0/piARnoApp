//
// Created by JW on 04/07/2022.
//

#include "Object.h"
#include "XrPassthroughGl.h"

using namespace OVR;


Object::Object(Geometry *geometry) : geometry(geometry) {

}

void Object::render() {
    //color per vertex (each corner)
    std::vector<unsigned char> colors(geometry->vertexCount * 4 / 3);
    for(size_t i = 0; i < colors.size(); i+=4) {
        colors[i+0] = r;
        colors[i+1] = g;
        colors[i+2] = b;
        colors[i+3] = a;
    }
    geometry->updateColors(std::move(colors));

    //set the transformation matrix and render
    auto trans =
            Matrix4f::Translation(posX, posY, posZ) *
             (Matrix4f::RotationZ(rotZ) * (Matrix4f::RotationY(rotY) * (Matrix4f::RotationX(rotX) * Matrix4f::Scaling(sclX, sclY, sclZ))));

    geometry->render(trans);
}