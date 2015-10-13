#pragma once

#include "ofMain.h"
#include "ofAgingMesh.h"
#include "addons/ofxAssimpModelLoader/src/ofxAssimpModelLoader.h"

#include "chuck_fft.h"
#include "audio_input.h"

class ofApp: public ofBaseApp {
public:
    ofApp(float width, float height);
    ~ofApp();
    
    void setup();
    void update();
    void draw();

    void keyPressed(int key);
    void keyReleased(int key);
    void windowResized(int w, int h);
    
private:
    bool keyLeft, keyRight, keyUp, keyDown;
    int sceneIndex = 0;
    ofTrueTypeFont font;
    
    /* Draws the scene without any post-processing effects. */
    void drawScene(int sceneIndex, bool flush);
    
    /* Draws post-processing effects on top of the scene. */
    void postProcessScene(bool flush);
    
    /* Creates the ship. */
    ofMesh createShip();
    
    /* Creates a box on the audio highway. */
    ofAgingMesh createBox(float* signal, size_t signalLength);
    
    /* Creates a road chunk on the audio highway. */
    ofAgingMesh createRoadChunk(float* signal, size_t signalLength);
    
    /* Creates a circular outline in the audio highway. */
    ofAgingMesh createTunnelChunk(float* signal, size_t signalLength);
    
    /* Creates a line strip mesh visualizing the instantaneous sound wave. */
    ofAgingMesh createTimeDomainMesh(float* signal, size_t signalLength);
    
    /* Creates a line strip mesh visualizing the frequency spectrum of the
     * instantenous sound wave. */
    ofAgingMesh createFrequencySpectrumMesh(complex* spectrum, size_t spectrumSize);
    
    /* Current window width, height. */
    float windowWidth;
    float windowHeight;
    
    /* Audio input library. */
    AudioInput audio;
    
    /* Blur shader and support variables. */
    ofFbo sceneBuffer;
    ofShader horizontalBlurShader;
    ofShader verticalBlurShader;
    ofFbo firstPassBuffer;
    ofFbo secondPassBuffer;
    ofMesh screen;
    
    /* Ship model and location variables. */
    ofMesh ship;
    ofVec3f modelPosition;
    ofVec3f modelRotationalAcceleration;
    ofVec3f modelRotationalVelocity;
    ofVec3f modelRotation;
    ofVec3f modelPerturbation;
    
    /* Vectors of all generated time domain meshes and frequency spectrum
     * meshes. */
    std::vector<ofAgingMesh> boxes;
    std::vector<ofAgingMesh> roadChunks;
    std::vector<ofAgingMesh> tunnelChunks;
    ofAgingMesh timeMesh;
    std::vector<ofAgingMesh> frequencyMeshes;
};
