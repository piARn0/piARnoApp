//
// Created by JW on 04/07/2022.
//

#include "Object.h"
#include "Engine.h"

using namespace global;

color::color(color_t r, color_t g, color_t b, color_t a, Geometry *perVertexGeom) : data{r, g, b, a} {
    if(perVertexGeom && !perVertexGeom->global_color) {
        data.resize(perVertexGeom->vertexCount * 4 / 3);
        for (size_t i = 4; i < data.size(); i += 4) {
            data[i + 0] = r;
            data[i + 1] = g;
            data[i + 2] = b;
            data[i + 3] = a;
        }
    }
}

color_t& color::r(int i) {
    return data[4*i + 0];
}

color_t& color::g(int i) {
    return data[4*i + 1];
}

color_t& color::b(int i) {
    return data[4*i + 2];
}

color_t& color::a(int i) {
    return data[4*i + 3];
}

void color::setAll(size_t channel, color_t val) {
    for(size_t i = channel; i < data.size(); i += 4)
        data[i] = val;
}


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
    if(!show)
        return;

    geometry->updateColors(col.data);

    //set the transformation matrix and render
    mat4 trans = translate(pos) * rotate(rot) * scale(scl);
    if(postTransform)
        geometry->render(*postTransform * trans);
    else
        geometry->render(trans);
}

vec3 Object::globalPos(std::optional<vec3> p) const {
    if(parent)
        return (translate(parent->pos) * rotate(parent->rot) * scale(parent->scl)).Transform(p.value_or(pos));
    else
        return p.value_or(pos);
}

vec3 Object::globalRot(std::optional<vec3> r) const {
    if(parent)
        //FIXME: this is correct... right???
        return parent->rot + r.value_or(rot);
    else
        return r.value_or(rot);
}

vec3 Object::globalScl(std::optional<vec3> s) const {
    if(parent)
        return parent->scl * s.value_or(scl);
    else
        return s.value_or(scl);
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

void ObjectGroup::detach(Object *ptr) {
    auto it = std::find(objects.begin(), objects.end(), ptr);
    if(it != objects.end())
        objects.erase(it);
}


Rigid::Rigid(Geometry *geometry) : Object(geometry) {

}

bool Rigid::isColliding(const Rigid &other) {
    vec3 a = globalPos(pos + offset);
    vec3 b = other.globalPos(other.pos + other.offset);
    float radiusSq = (radius + other.radius) * (radius + other.radius);

    return a.DistanceSq(b) <= radiusSq;
}


Button::Button(Geometry *geometry) : Rigid(geometry) {

}

void Button::update(const std::vector<Rigid> &controllers) {
    pressedPrev = pressed;

    float currentPress = 0;
    vec3 p1 = globalPos();
    for(auto &c : controllers) {
        vec3 p2 = c.globalPos();
        if(isColliding(c)) { //is pushing down from above
            //calculate press distance
            float radiusSq = (radius + c.radius) * (radius + c.radius);
            float horizontalDistSq = (p1.x - p2.x) * (p1.x - p2.x) + (p1.z - p2.z) * (p1.z - p2.z);
            float verticalDist = p2.y - p1.y;
            float pressDist = sqrt(radiusSq - horizontalDistSq) - verticalDist - 0.01;
            currentPress = std::max(currentPress, pressDist);
        }
    }

    currentPress = std::min(currentPress, maxPress);
    pressed = currentPress >= maxPress / 2;
    offset.y = -currentPress;
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
    if(!show)
        return;

    color c = pressed && pressCol.a() > 0 ? pressCol : col;
    geometry->updateColors(c.data);


    //set the transformation matrix and render
    mat4 trans = translate(pos + offset) * rotate(rot) * scale(scl);
    if(postTransform)
        geometry->render(*postTransform * trans);
    else
        geometry->render(trans);

    if(!label.empty()) {
        auto p = globalPos(pos + offset + vec3{0, scl.y, 0}), s = globalScl(scl * vec3{0.5, 0.8, 1}), r = globalRot(rot + vec3{-M_PI/2, labelRot, 0});
        engine->renderText(label, p, s, r, color{255, 255, 255, 200});
    }
}


Slider::Slider(Geometry *geometry) : Button(geometry) {
}

Slider::Slider(float minVal, float value, float maxVal, float left, float right, Geometry *geometry)
: Button(geometry), minVal(minVal), maxVal(maxVal), min(left), max(right) {
    set(value);
}


void Slider::update(const std::vector<Rigid> &controllers) {
    pressedPrev = pressed;

    float currentPress = 0;
    int controllerIndex = -1;
    vec3 p1 = globalPos(pos + vec3{offset.x, 0, offset.z});
    for(size_t i = 0; i < controllers.size(); i++) {
        auto &c = controllers[i];
        vec3 p2 = c.globalPos();
        if(isColliding(c)) { //is pushing down from above
            //calculate press distance
            float radiusSq = (radius + c.radius) * (radius + c.radius);
            float horizontalDistSq = (p1.x - p2.x) * (p1.x - p2.x) + (p1.z - p2.z) * (p1.z - p2.z);
            float verticalDist = p2.y - p1.y;
            float pressDist = sqrt(radiusSq - horizontalDistSq) - verticalDist - 0.01;

            if(pressDist > currentPress) {
                controllerIndex = i;
                currentPress = pressDist;
            }
        }
    }

    currentPress = std::min(currentPress, maxPress);
    pressed = currentPress >= maxPress / 2;

    if(pressed && !pressedPrev) {
        controllerOffset = calculateOffset(controllers[controllerIndex].pos) - offset;
    }

    if(controllerIndex != -1 && pressed) {
        offset = vec3::Max(min * trackDir, vec3::Min(calculateOffset(controllers[controllerIndex].pos) - controllerOffset, max * trackDir));
        val = sqrt((offset.x * offset.x) + (offset.z * offset.z));
    }
    offset.y = -currentPress;
}

float Slider::get() {
    return minVal + val / (max - min) * (maxVal - minVal);
}

void Slider::set(float v) {
    val = (max-min) * (v-minVal) / (maxVal-minVal);
    auto newOffset = val * trackDir;
    offset.x = newOffset.x;
    offset.z = newOffset.z;
}

void Slider::render(mat4 *postTransform) {
    if(!show)
        return;

    Button::render(postTransform);

    //render slider track
    auto track = engine->getGeometry(Mesh::rect);
    track->updateColors(color{50, 50, 50, 255}.data);

    //set the transformation matrix and render
    mat4 trans = translate(pos + vec3{(min+max)/2, -scl.y/2, 0}) * rotate(rot + vec3{(float)-M_PI/2, 0, 0}) * scale(vec3{max - min, 0.01, 1});
    if(postTransform)
        track->render(*postTransform * trans);
    else
        track->render(trans);
}

vec3 Slider::calculateOffset(vec3 controllerPos) {
    vec3 c = (controllerPos - globalPos());
    vec3 r = globalRot();
    vec3 relative{c.x * cos(-r.y) + c.z * sin(-r.y), c.y, -c.x * sin(-r.y) + c.z * cos(-r.y)}; //rotate controller pos around y axis to match slider rotation
    return relative.ProjectTo(trackDir);
}