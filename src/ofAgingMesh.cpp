#include "ofAgingMesh.h"

ofAgingMesh::ofAgingMesh() : ofMesh() {
    birthdate = ofGetElapsedTimef();
}

ofAgingMesh::ofAgingMesh(float maxAge) : ofMesh() {
    birthdate = ofGetElapsedTimef();
    maxAgeInSeconds = maxAge;
}

bool ofAgingMesh::isAlive() {
    return getAge() < maxAgeInSeconds;
}

float ofAgingMesh::getAgePercent() {
    return (maxAgeInSeconds - getAge()) / maxAgeInSeconds;
}

float ofAgingMesh::getAge() {
    return ofGetElapsedTimef() - birthdate;
}