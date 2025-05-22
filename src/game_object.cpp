#include "../include/entities.h"
#include "../include/utils.h"
#include <iostream>
#include <string>
#include <cmath>
#include <vector>

using namespace std;

GameObject::GameObject(Vector2D pos, Vector2D dims, Vector2D dir, 
                        Scope scope,
                        Uint8 r, Uint8 g, Uint8 b, Uint8 a, int speed 
                        ):
    position(pos), 
    dimensions(dims), 
    direction(dir), 
    scope(scope),
    speed(speed), 
    isActive(true), 
    angle(0.0f) 
{
    color[0] = r; color[1] = g; color[2] = b; color[3] = a;
}

GameObject::~GameObject() {
    texture = nullptr;
}

// --- movement -------------------------------------------------
void GameObject::setPosition(const Vector2D& pos) {
    position = pos;
}

void GameObject::setDirection(const Vector2D& dir) {
    direction = dir;
    // moving direction should be independent of angle
}

void GameObject::move(Vector2D delta) {
    position = position + delta;
    updateCollisionVertices();
}

// --- dimensions and rotation -------------------------------
void GameObject::setDimensions(const Vector2D& dims) {
    dimensions = dims;
    updateCollisionVertices();
    // what this for
}

void GameObject::setAngle(float angle) {
    this->angle = angle;
    updateCollisionVertices();
}

void GameObject::rotate(float dAngle) {
    angle += dAngle;
    updateCollisionVertices();

}

// --- state management --------------------------------------
void GameObject::setActive(bool active) {
    isActive = active;
}

void GameObject::setScope(Scope scope) {
    this->scope = scope;
    // not used
}

// --- bounds detection --------------------------------------
bool GameObject::isOutOfBounds(const SDL_Rect& bounds) const {
    return (
        position.x < bounds.x || 
        position.x > bounds.x + bounds.w ||
        position.y < bounds.y || 
        position.y > bounds.y + bounds.h
    );
}

bool GameObject::isInWindow(const SDL_Rect& bounds) const {
    return (
        position.x > bounds.x && 
        position.x < bounds.x + bounds.w &&
        position.y > bounds.y && 
        position.y < bounds.y + bounds.h
    );
}

// --- getters -----------------------------------------------
Vector2D GameObject::getPosition() const {return position;}
Vector2D GameObject::getDimensions() const {return dimensions;}
Vector2D GameObject::getDirection() const {return direction;}
float    GameObject::getAngle() const {return angle;}
GameObject::Scope GameObject::getScope() const {return scope;}
bool     GameObject::getActive() const {return isActive;}

// --- vertices ----------------------------------------------
std::vector<Vector2D> GameObject::getCollisionVertices() const {
    return vertices;
}

void GameObject::setCollisionVertices(const std::vector<Vector2D>& vertices) {
    this->vertices = vertices;
}

void GameObject::initRectangleCollision() {
    localVertices = {
        Vector2D(-dimensions.x / 2, -dimensions.y / 2),
        Vector2D(+dimensions.x / 2, -dimensions.y / 2),
        Vector2D(+dimensions.x / 2, +dimensions.y / 2),
        Vector2D(-dimensions.x / 2, +dimensions.y / 2)
    }; // doesn't really need the plus sign but it's there
    updateCollisionVertices();
}

void GameObject::initCircleCollision() {
    // magic number
    // todo: how many vertices is a "good enough" approximation?
    int numVertices = 12; // approximation
    vertices.clear();
    localVertices.clear();
    for (int i = 0; i < numVertices; i++) {
        float a = (2 * M_PI / numVertices) * i;
        Vector2D vertex {
            (dimensions.x / 2.0f) * cos(a),
            (dimensions.y / 2.0f) * sin(a)
        };
        localVertices.push_back(vertex);
    }
    updateCollisionVertices();
}

void GameObject::updateCollisionVertices() {
    vertices.clear();
    float c = cos(angle), s = sin(angle);
    for (auto& vertex : localVertices) {
        float x = vertex.x * c - vertex.y * s;
        float y = vertex.x * s + vertex.y * c;
        vertices.push_back(Vector2D(x, y) + position);
    }
}

// --- sat implementation -----------------------------------
bool GameObject::checkCollision(const GameObject& other) {
    return checkSATCollision(vertices, other.vertices);
}

vector<Vector2D> GameObject::getAxes(const std::vector<Vector2D>& vertices) {
    vector<Vector2D> axes;
    size_t numVertices = vertices.size();
    for (size_t i = 0; i < numVertices; i++) {
        Vector2D p1 = vertices[i];
        Vector2D p2 = vertices[(i + 1) % numVertices];
        Vector2D edge = p2 - p1;
        Vector2D normal = Vector2D(-edge.y, edge.x);
        normal.normalize();
        axes.push_back(normal);
    }
    return axes;
}

void GameObject::project(
    const vector<Vector2D>& vertices,
    const Vector2D& axis,
    float& minOut, float& maxOut) 
{
    minOut = 1e38f; maxOut = -1e38f;
    if (vertices.empty()) {
        cerr << "Error: No vertices to project." << endl;
        return;
    }
    for (const auto& vertex : vertices) {
        float projection = vertex.dot(axis);
        minOut = min(minOut, projection);
        maxOut = max(maxOut, projection);
    }
}

bool GameObject::checkSATCollision(
    const vector<Vector2D>& vertices1,
    const vector<Vector2D>& vertices2)
{
    vector<Vector2D> axes1 = getAxes(vertices1), axes2 = getAxes(vertices2);
    vector<Vector2D> axes;
    axes.insert(axes.end(), axes1.begin(), axes1.end());
    axes.insert(axes.end(), axes2.begin(), axes2.end());
    
    for (auto& axis : axes) {
        float min1, max1, min2, max2;
        project(vertices1, axis, min1, max1);
        project(vertices2, axis, min2, max2);
        
        if (max1 < min2 || max2 < min1) {
            return false; // no collision, exit
        }
    }
    return true; // collision detected
}