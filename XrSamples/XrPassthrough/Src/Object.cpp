//
// Created by JW on 04/07/2022.
//

#include "Object.h"

mat4 translate(vec3 pos) {
    return mat4::Translation(pos);
}

mat4 scale(vec3 scl) {
    return mat4::Scaling(scl);
}

mat4 rotate(vec3 rot) {
    return mat4::RotationZ(rot.z) * mat4::RotationY(rot.y) * mat4::RotationX(rot.x);
}


Object::Object(Geometry *geometry) : geometry(geometry) {

}

void Object::render() {
    if (geometry->global_color) {
        //color of whole object
        geometry->updateColors(std::vector<color_t> {col.r, col.g, col.b, col.a});
    } else {
        //color each vertex the same color
        std::vector<color_t> colors(geometry->vertexCount * 4 / 3);
        for (size_t i = 0; i < colors.size(); i += 4) {
            colors[i + 0] = col.r;
            colors[i + 1] = col.g;
            colors[i + 2] = col.b;
            colors[i + 3] = col.a;
        }
        geometry->updateColors(colors);
    }

    //set the transformation matrix and render
    mat4 trans = translate(pos) * rotate(rot) * scale(scl);

    geometry->render(trans);
}

void Object::render(const mat4 &trans) {
    if (geometry->global_color) {
        //color of whole object
        geometry->updateColors(std::vector<color_t> {col.r, col.g, col.b, col.a});
    } else {
        //color each vertex the same color
        std::vector<color_t> colors(geometry->vertexCount * 4 / 3);
        for (size_t i = 0; i < colors.size(); i += 4) {
            colors[i + 0] = col.r;
            colors[i + 1] = col.g;
            colors[i + 2] = col.b;
            colors[i + 3] = col.a;
        }
        geometry->updateColors(colors);
    }

    geometry->render(trans);
}

ObjectGroup::ObjectGroup()
{}

void ObjectGroup::render() {
    mat4 trans = translate(pos) * rotate(rot) * scale(scl);
    for(auto &o : objects) {
        o.render(trans * translate(o.pos) * rotate(o.rot) * scale(o.scl));
    }
}

Object& ObjectGroup::operator[](size_t i) {
    return objects[i];
}


Rigid::Rigid(Geometry *geometry) : Object(geometry) {

}

bool Rigid::doesCollide(const Rigid &other) {
//    float midX = posX + offsetX, midY = posY + offsetY, midZ = posZ + offsetZ;
//    float x1 =
    return false;
}


Button::Button(Geometry *geometry) : Rigid(geometry) {

}