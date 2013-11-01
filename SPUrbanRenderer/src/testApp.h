/**
 * Example - Scene With Timeline
 * This example shows how to create a basic RGB+D scene
 * with a timeline to scrub through the video.
 *
 * It requiers ofxGameCamera https://github.com/Flightphase/ofxGameCamera 
 * and ofxTimeline https://github.com/Flightphase/ofxTimeline in addition to ofxRGBDepth
 *
 * James George 2012 
 * Released under the MIT License
 *
 * The RGBDToolkit has been developed with support from the STUDIO for Creative Inquiry and Eyebeam
 */

#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxGameCamera.h"
#include "ofxRGBDPlayer.h"
#include "ofxRGBDGPURenderer.h"
#include "ofxRGBDCPURenderer.h"
#include "ofxSyphon.h"

#include "PortraitScreen.h"

#include "ofxGui.h"

class testApp : public ofBaseApp{
  public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    void exit();
    
	void generateGeometry();
	
    ofxRGBDPlayer player;
    ofxRGBDGPURenderer renderer;
	
	int currentPortraitIndex;
	vector<string> paths;
	void gotoNextPortrait();
	void gotoPreviousPortrait();
	void switchPortrait();
	float nextPortraitTime;
	float portraitChangedTime;
	
	ofRectangle depthRect;
	
    ofxPanel gui;
    ofxFloatSlider xrotate;
    ofxFloatSlider yrotate;
	ofxFloatSlider zclip;
	ofxFloatSlider timePerPortrait;
	ofxFloatSlider flowSpeed;

	ofxFloatSlider maxExtendLED;
	ofxFloatSlider maxExtendFacade;
	
	ofxFloatSlider extendThreshold;
	ofxFloatSlider extendFalloff;

	ofxFloatSlider headSphereRadius;
	ofxFloatSlider headEffectFalloff;
	ofxFloatSlider varianceEffectLED;
	ofxFloatSlider varianceEffectFacade;
	ofxFloatSlider pureColorThreshold;
	ofxFloatSlider pureColorFlicker;
	float pureColorFlickerPos;
	
//    bool loadNewScene();
//    bool loadDefaultScene();
    bool loadScene(string takeDirectory);

	void saveHeadPositions();
	void loadHeadPositions();
	void loadPalettes();
	void selectPalette();
	int currentPalette;
	
	map<string, ofVec2f> headPositions;
	
	vector<PortraitScreen*> screens;
	PortraitScreen* highlightScreen;
	PortraitScreen led1;
	PortraitScreen led2;

	PortraitScreen leftFacade;
	PortraitScreen centerFacade;
	PortraitScreen rightFacade;
	
	ofVboMesh mesh;
	ofVboMesh pointfill;
	ofFloatImage varianceImage;
	ofFloatImage speedVarianceImage;
	
	ofFbo fbo;
	ofxSyphonServer syphonServer;
	vector<ofImage> colorPalettes;
	ofMatrix4x4 colormatrix;
	
};
