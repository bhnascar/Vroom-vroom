#pragma once

#include "ofMain.h"

/* Wrapper class around ofMesh that gives a mesh an
 * age in elapsed time. */
class ofAgingMesh : public ofMesh {
public:
    ofAgingMesh();
    ofAgingMesh(float maxAge);
    
    /* Returns true if theis mesh is alive. */
    bool isAlive();
    
    /* Get the age of this mesh in seconds. */
    float getAge();
    
    /* Get the age of this mesh in percent of expected lifetime. */
    float getAgePercent();
    
private:
    /* The birthdate of this mesh, in elapsed seconds since
     * the start of the application. */
    float birthdate;
    
    /* The age after which the mesh is considered dead. */
    float maxAgeInSeconds;
};

