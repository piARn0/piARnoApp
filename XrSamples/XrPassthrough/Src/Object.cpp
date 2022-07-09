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

void Object::render(mat4 *postTransform) {
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
    if(postTransform)
        geometry->render(*postTransform * trans);
    else
        geometry->render(trans);
}

vec3 Object::globalPos() const {
    if(parent)
        return (translate(parent->pos) * rotate(parent->rot) * scale(parent->scl)).Transform(pos);
    else
        return pos;
}

vec3 Object::globalRot() const {
    if(parent)
        //FIXME: this is correct... right???
        return parent->rot + rot;
    else
        return rot;
}

vec3 Object::globalScl() const {
    if(parent)
        return parent->scl * scl;
    else
        return scl;
}

ObjectGroup::ObjectGroup()
{}

void ObjectGroup::render() {
    mat4 trans = translate(pos) * rotate(rot) * scale(scl);
    for(auto o : objects) {
        o->render(&trans);
    }
}

Object& ObjectGroup::operator[](size_t i) {
    return *objects[i];
}

ObjectGroup::iterator ObjectGroup::begin() {
    return objects.begin();
}

ObjectGroup::iterator ObjectGroup::end() {
    return objects.end();
}

size_t ObjectGroup::size() {
    return objects.size();
}

void ObjectGroup::attach(Object &obj) {
    objects.push_back(&obj);
    obj.parent = this;
}


Rigid::Rigid(Geometry *geometry) : Object(geometry) {

}

bool Rigid::isColliding(const Rigid &other) {
    vec3 a = globalPos();
    vec3 b = other.globalPos();
    float radiusSq = (radius + other.radius) * (radius + other.radius);

    return a.DistanceSq(b) <= radiusSq;
}


Button::Button(Geometry *geometry) : Rigid(geometry) {

}

void Button::update(const std::vector<Rigid> &controllers) {
    pressedPrev = pressed;
    currentPress = 0;

    vec3 p1 = globalPos();
    for(auto &c : controllers) {
        vec3 p2 = c.globalPos();
        if(p2.y > p1.y && isColliding(c)) { //is pushing down from above
            //calculate press distance
            float radiusSq = (radius + c.radius) * (radius + c.radius);
            float horizontalDistSq = (p1.x - p2.x) * (p1.x - p2.x) + (p1.z - p2.z) * (p1.z - p2.z);
            float verticalDist = p2.y - p1.y;
            float pressDist = sqrt(radiusSq - horizontalDistSq) - verticalDist;
            currentPress = std::max(currentPress, pressDist);
        }
    }

    currentPress = std::min(currentPress, maxPress);
    pressed = currentPress >= maxPress / 2;

    //TODO: vibration for feedback?
}

bool Button::isPressed() {
    return pressed && !pressedPrev;
}

bool Button::isReleased() {
    return !pressed && pressedPrev;
}

bool Button::isBeingPressed() {
    return pressed;
}

void Button::render(mat4 *postTransform) {
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
    mat4 trans = translate(vec3{pos.x, pos.y - currentPress, pos.z}) * rotate(rot) * scale(scl);
    if(postTransform)
        geometry->render(*postTransform * trans);
    else
        geometry->render(trans);
}