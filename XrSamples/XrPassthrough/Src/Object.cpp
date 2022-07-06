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