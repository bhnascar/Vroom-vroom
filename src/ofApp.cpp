#include "ofApp.h"

ofApp::ofApp(float width, float height)
    : audio(44100, 1024), windowWidth(width), windowHeight(height) {
    audio.Start();
}

ofApp::~ofApp() {
    audio.Stop();
}

void ofApp::setup() {
    // Load and compile shaders
    horizontalBlurShader.load("blur.vert", "blurX.frag");
    if (!horizontalBlurShader.linkProgram()) {
        std::cerr << "Shaders not set up correctly!" << std::endl;
    }
    verticalBlurShader.load("blur.vert", "blurY.frag");
    if (!verticalBlurShader.linkProgram()) {
        std::cerr << "Shaders not set up correctly!" << std::endl;
    }
    
    // Create new framebuffers
    sceneBuffer.allocate(windowWidth, windowHeight, GL_RGBA);
    firstPassBuffer.allocate(windowWidth, windowHeight, GL_RGBA);
    secondPassBuffer.allocate(windowWidth, windowHeight, GL_RGBA);
    if (!sceneBuffer.checkStatus() || !firstPassBuffer.checkStatus() || !secondPassBuffer.isAllocated()) {
        std::cerr << "Framebuffers not set up correctly!" << std::endl;
    }
    
    // Load font
    font.loadFont("Tahoma.ttf", 18, true, true);
    
    // Create ship.
    ship = createShip();
    modelPosition = ofVec3f(windowWidth / 2.f, windowHeight / 2.f + 200, 0.f);
    keyUp = keyDown = keyLeft = keyRight = false;
}

ofMesh ofApp::createShip() {
    // Load object file.
    ofxAssimpModelLoader model;
    model.loadModel("ship.obj");
    
    // Get mesh faces.
    const std::vector<ofMeshFace>& faces(model.getMesh(0).getUniqueFaces());
    ofMesh fixedMesh;
    fixedMesh.setMode(OF_PRIMITIVE_LINES);

    // Scale mesh.
    for( int i = 0; i < faces.size(); i++ ) {
        ofMeshFace face = faces[i];
        fixedMesh.addVertex(50.f * face.getVertex(0));
        fixedMesh.addVertex(50.f * face.getVertex(1));
        fixedMesh.addVertex(50.f * face.getVertex(1));
        fixedMesh.addVertex(50.f * face.getVertex(2));
        fixedMesh.addVertex(50.f * face.getVertex(2));
        fixedMesh.addVertex(50.f * face.getVertex(0));
    }
    
    return fixedMesh;
}

ofAgingMesh ofApp::createBox(float *signal, size_t signalLength) {
    ofAgingMesh mesh(3.0f);
    mesh.setMode(OF_PRIMITIVE_LINE_LOOP);
    
    float width = 200.f;
    float radius = max(windowWidth / 2, windowHeight / 2) * audio.GetCurrentAmplitude() / 0.3f;
    float angle = ofRandom(0, M_PI) * audio.GetCurrentPitch() / (audio.GetFrequencyResolution() / 10.f);
    float topX = windowWidth / 2 + radius * cos(angle) - width / 2.f;
    float topY = windowHeight / 2 - radius * sin(angle) - width / 2.f;
    
    mesh.addVertex(ofVec3f(topX, topY, 0.f));
    mesh.addVertex(ofVec3f(topX + width, topY, 0.f));
    mesh.addVertex(ofVec3f(topX + width, topY + width, 0.f));
    mesh.addVertex(ofVec3f(topX, topY + width, 0.f));
    
    return mesh;
}

ofAgingMesh ofApp::createRoadChunk(float *signal, size_t signalLength) {
    ofAgingMesh mesh(3.0f);
    mesh.setMode(OF_PRIMITIVE_LINE_LOOP);
    
    float radius = max(windowWidth / 2, windowHeight / 2);
    float offset = min(100.f, 200.f * audio.GetCurrentAmplitude());
    float leftX = windowWidth / 2 + radius * cos(M_PI + 0.5);
    float leftY = windowHeight / 2 - radius * sin(M_PI + 0.5) - offset;
    float rightX = windowWidth / 2 + radius * cos(-0.5);
    float rightY = windowHeight / 2 - radius * sin(-0.5) - offset;
    float width = 500.f;
    
    mesh.addVertex(ofVec3f(leftX, leftY, 0.f));
    mesh.addVertex(ofVec3f(rightX, rightY, 0.f));
    mesh.addVertex(ofVec3f(rightX, rightY, width));
    mesh.addVertex(ofVec3f(leftX, leftY, width));
    
    return mesh;
}


ofAgingMesh ofApp::createTunnelChunk(float* signal, size_t signalLength) {
    ofAgingMesh mesh(3.0f);
    mesh.setMode(OF_PRIMITIVE_LINE_LOOP);
    
    float radius = max(windowWidth / 1.5, windowHeight / 1.5);
    
    float start = 50.f;
    float end = windowWidth - 50.f;
    float increment = (end - start) / signalLength;
    for (size_t i = 0; i < signalLength; i++) {
        float adjustedRadius = radius + 100 * signal[i];
        float x = windowWidth / 2 + adjustedRadius * cos((float)i / signalLength * 2 * (M_PI - 0.01));
        float y = windowHeight / 2 + adjustedRadius * sin((float)i / signalLength * 2 * (M_PI - 0.01));
        float z = 0.f;
        mesh.addVertex(ofVec3f(x, y, z));
    }
    
    return mesh;
}

ofAgingMesh ofApp::createTimeDomainMesh(float* signal, size_t signalLength) {
    ofAgingMesh mesh(3.0f);
    mesh.setMode(OF_PRIMITIVE_LINE_STRIP);
    
    float start = 50.f;
    float end = windowWidth - 50.f;
    float increment = (end - start) / signalLength;
    for (size_t i = 0; i < signalLength; i++) {
        float x = start + i * increment;
        float y = windowHeight / 4.f + 100.f * signal[i];
        float z = 0.f;
        mesh.addVertex(ofVec3f(x, y, z));
    }
    
    return mesh;
}

ofAgingMesh ofApp::createFrequencySpectrumMesh(complex* spectrum, size_t spectrumSize) {
    ofAgingMesh mesh(1.0f);
    mesh.setMode(OF_PRIMITIVE_LINE_STRIP);
    
    float start = 50.f;
    float end = windowWidth - 50.f;
    float increment = (end - start) / spectrumSize;
    for (size_t i = 0; i < spectrumSize; i++) {
        float x = start + i * increment;
        float y = 3 * windowHeight / 4.f - 200.f * ::pow(25 * cmp_abs(spectrum[i]), 0.5);
        float z = 0.f;
        mesh.addVertex(ofVec3f(x, y, z));
    }
    
    return mesh;
}

void ofApp::update() {
    // Move older meshes back.
    for (int i = 0; i < roadChunks.size(); i++) {
        if (!roadChunks[i].isAlive()) {
            roadChunks.erase(roadChunks.begin() + i--);
        }
    }
    for (int i = 0; i < boxes.size(); i++) {
        if (!boxes[i].isAlive()) {
            boxes.erase(boxes.begin() + i--);
        }
    }
    for (int i = 0; i < tunnelChunks.size(); i++) {
        if (!tunnelChunks[i].isAlive()) {
            tunnelChunks.erase(tunnelChunks.begin() + i--);
        }
    }
    for (int i = 0; i < frequencyMeshes.size(); i++) {
        if (!frequencyMeshes[i].isAlive()) {
            frequencyMeshes.erase(frequencyMeshes.begin() + i--);
        }
    }

    static int count = 0;
    float* buffer = audio.GetCurrentInput();
    
    // Create tunnel chunk.
    if (count++ % 10 == 0) {
        ofAgingMesh tunnelChunk = createTunnelChunk(buffer, 1024);
        tunnelChunks.push_back(tunnelChunk);
    }
    
    // Create road chunk.
    if (count % 30 == 0) {
        ofAgingMesh roadChunk = createRoadChunk(buffer, 1024);
        roadChunks.push_back(roadChunk);
    }
    
    // Create box.
    if (count % 30 == 0) {
        ofAgingMesh box = createBox(buffer, 1024);
        boxes.push_back(box);
    }
    
    // Create timeMesh.
    timeMesh = createTimeDomainMesh(buffer, 1024);
    delete[] buffer;
    
    // Compute FFT. Add new frequency spectrum mesh.
    if (count % 5 == 0) {
        complex* frequencyBuffer = audio.GetTransformedInput();
        ofAgingMesh frequencyMesh = createFrequencySpectrumMesh(frequencyBuffer, 1024);
        frequencyMeshes.push_back(frequencyMesh);
        delete[] frequencyBuffer;
    }
    
    // Handle key presses.
    if (keyLeft) {
        modelPosition += ofVec3f(-10.f, 0.f, 0.f);
        modelRotationalAcceleration += ofVec3f(-50.f, 0.0f, 0.0f);
    }
    if (keyRight) {
        modelPosition += ofVec3f(10.f, 0.f, 0.f);
        modelRotationalAcceleration += ofVec3f(50.f, 0.0f, 0.0f);
    }
    if (keyDown) {
        modelPosition += ofVec3f(0.f, 10.f, 0.f);
        modelRotationalAcceleration += ofVec3f(0.f, -50.0f, 0.0f);
    }
    if (keyUp) {
        modelPosition += ofVec3f(0.f, -10.f, 0.f);
        modelRotationalAcceleration += ofVec3f(0.f, 50.0f, 0.0f);
    }
    modelRotationalAcceleration += 3.f * (ofVec3f(0.f, 0.f, 0.f) - modelRotation);
    modelRotationalAcceleration = 0.9 * modelRotationalAcceleration;
    modelRotationalVelocity = modelRotationalVelocity + ofGetLastFrameTime() * modelRotationalAcceleration;
    modelRotationalVelocity = 0.9 * modelRotationalVelocity;
    modelRotation = modelRotation + ofGetLastFrameTime() * modelRotationalVelocity;
    
    // Perturb model position.
    float time = ofGetElapsedTimef();
    modelPerturbation = ofVec3f(sin(2.f * time), cos(1.5f * time), 1.5 * sin(3.f * time));
}

void ofApp::drawScene(int sceneIndex, bool flush) {
    sceneBuffer.begin();
    
    // Enable depth testing.
    ofSetDepthTest(true);
    
    ofBackground(0, 0, 0);
    
    if (sceneIndex == 1) {
        // Draw ship.
        ofVec3f finalPosition = modelPosition + 10.f * modelPerturbation;
        ofPushMatrix();
        ofTranslate(finalPosition.x, finalPosition.y, finalPosition.z);
        ofRotate(modelRotation.x, 0, 0, 1);
        ofSetColor(255, 0, 255, 255);
        ship.draw();
        ofSetColor(255, 255, 255, 255);
        ofPopMatrix();
        
        // Draw tunnel chunks.
        static int count = 0;
        float frequency = 0.01;
        float red = sin(frequency * count + 0) * 127 + 128;
        float green = sin(frequency * count + 2) * 127 + 128;
        float blue = sin(frequency * count + 4) * 127 + 128;
        ofColor tunnelColor(red, green, blue);
        for (int i = 0; i < tunnelChunks.size(); i++) {
            ofPushMatrix();
            ofTranslate(0, 0, -5000.f * tunnelChunks[i].getAgePercent() + 200.f);
            ofColor color = tunnelColor * (1.f - tunnelChunks[i].getAgePercent());
            ofSetColor(color);
            tunnelChunks[i].draw();
            ofSetColor(255, 255, 255, 255);
            ofPopMatrix();
        }
        count++;
        
        // Draw road chunks.
        for (int i = 0; i < roadChunks.size(); i++) {
            ofPushMatrix();
            ofTranslate(0, 0, -5000.f * roadChunks[i].getAgePercent() + 200.f);
            if (flush) {
                ofEnableAlphaBlending();
                ofSetColor(0, 200.f * (1.f - roadChunks[i].getAgePercent()), 255.f * (1.f - roadChunks[i].getAgePercent()), 20);
                roadChunks[i].setMode(OF_PRIMITIVE_TRIANGLE_FAN);
                roadChunks[i].draw();
                ofSetColor(255, 255, 255, 255);
                ofDisableAlphaBlending();
            }
            ofSetColor(0, 200.f * (1.f - roadChunks[i].getAgePercent()), 255.f * (1.f - roadChunks[i].getAgePercent()), 255);
            roadChunks[i].setMode(OF_PRIMITIVE_LINE_LOOP);
            roadChunks[i].draw();
            ofSetColor(255, 255, 255, 255);
            ofPopMatrix();
        }
        
        // Draw boxes.
        for (int i = boxes.size() - 1; i >= 0; i--) {
            ofPushMatrix();
            ofTranslate(0, 0, -5000.f * boxes[i].getAgePercent() + 200.f);
            if (flush) {
                ofEnableAlphaBlending();
                ofSetColor(255.f * (1.f - boxes[i].getAgePercent()), 200.f * (1.f - boxes[i].getAgePercent()), 0, 20);
                boxes[i].setMode(OF_PRIMITIVE_TRIANGLE_FAN);
                boxes[i].draw();
                ofSetColor(255, 255, 255, 255);
                ofDisableAlphaBlending();
            }
            ofSetColor(200.f * (1.f - boxes[i].getAgePercent()), 200.f * (1.f - boxes[i].getAgePercent()), 0, 255);
            boxes[i].setMode(OF_PRIMITIVE_LINE_LOOP);
            boxes[i].draw();
            ofSetColor(255, 255, 255, 255);
            ofPopMatrix();
        }
    }
    else {
        // Draw instantaneous sound signal.
        ofSetColor(0, 255, 0, 255);
        timeMesh.draw();
        ofSetColor(255, 255, 255, 255);
        
        // Draw Fourier transformed signal.
        for (int i = 0; i < frequencyMeshes.size(); i++) {
            ofPushMatrix();
            ofTranslate(0, 0, -500.f * (1.f - frequencyMeshes[i].getAgePercent()));
            ofSetColor(0, 200.f * frequencyMeshes[i].getAgePercent(), 255.f * frequencyMeshes[i].getAgePercent(), 255);
            frequencyMeshes[i].draw();
            ofSetColor(255, 255, 255, 255);
            ofPopMatrix();
        }
    }
    
    // Disable depth testing.
    ofSetDepthTest(false);
    
    sceneBuffer.end();
    
    // Draw contents of framebuffer to output.
    if (flush) {
        sceneBuffer.getTextureReference().draw(0, 0, windowWidth, windowHeight);
    }
}

void ofApp::postProcessScene(bool flush) {
    // Perform horizontal blur. We do 4 passes to reduce banding artifacts.
    for (int i = 0; i < 4; i++) {
        ofFbo& renderBuffer = (i % 2 == 0) ? firstPassBuffer : sceneBuffer;
        ofFbo& textureBuffer = (i % 2 == 0) ? sceneBuffer : firstPassBuffer;
        
        // Perform horizontal blur.
        renderBuffer.begin();
        horizontalBlurShader.begin();
        horizontalBlurShader.setUniform1f("blurAmnt", 1.f);
        ofBackground(0, 0, 0);
        textureBuffer.getTextureReference().draw(0, 0, windowWidth, windowHeight);
        horizontalBlurShader.end();
        renderBuffer.end();
    }
    
    // Perform vertical blur. We do 4 passes to reduce banding artifacts.
    for (int i = 0; i < 4; i++) {
        ofFbo& renderBuffer = (i % 2 == 0) ? secondPassBuffer : sceneBuffer;
        ofFbo& textureBuffer = (i % 2 == 0) ? sceneBuffer : secondPassBuffer;
        
        renderBuffer.begin();
        verticalBlurShader.begin();
        verticalBlurShader.setUniform1f("blurAmnt", 1.f);
        ofBackground(0, 0, 0);
        textureBuffer.getTextureReference().draw(0, 0, windowWidth, windowHeight);
        verticalBlurShader.end();
        renderBuffer.end();
    }
    
    // Draw contents of framebuffer to output.
    if (flush) {
        sceneBuffer.getTextureReference().draw(0, 0, windowWidth, windowHeight);
    }
}

void ofApp::draw() {
    // Draw scene.
    drawScene(sceneIndex, true);
    
    // Add glow.
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    postProcessScene(true);
    ofDisableBlendMode();
    
    // Print framerate.
    std::ostringstream buff;
    buff << ofGetFrameRate();;
    font.drawString(buff.str(), 10, 30);
    
    // Print instructions.
    font.drawString("Press tab to change scenes.", windowWidth - 340, 30);
    if (sceneIndex == 1) {
        font.drawString("Use arrow keys to fly around!", windowWidth / 2 - 160, windowHeight - 15);
    }
}

void ofApp::keyPressed(int key) {
    switch(key) {
        case OF_KEY_LEFT:
            keyLeft = true;
            break;
        case OF_KEY_RIGHT:
            keyRight = true;
            break;
        case OF_KEY_UP:
            keyUp = true;
            break;
        case OF_KEY_DOWN:
            keyDown = true;
            break;
        case OF_KEY_TAB:
            sceneIndex = (sceneIndex) ? 0 : 1;
            break;
        default:
            break;
    }
}

void ofApp::keyReleased(int key) {
    switch(key) {
        case OF_KEY_LEFT:
            keyLeft = false;
            break;
        case OF_KEY_RIGHT:
            keyRight = false;
            break;
        case OF_KEY_UP:
            keyUp = false;
            break;
        case OF_KEY_DOWN:
            keyDown = false;
            break;
        default:
            break;
    }
}

void ofApp::windowResized(int w, int h) {
    windowWidth = w;
    windowHeight = h;
}
