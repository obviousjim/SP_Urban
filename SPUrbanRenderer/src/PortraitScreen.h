
#pragma once

#include "ofMain.h"
#include "ofxGameCamera.h"
#include "ofxGui.h"

class PortraitScreen {
  public:
	
	PortraitScreen();
	
	PortraitScreen* pair;
	
	void saveSettings();
	void drawDebug();
	
	ofVec2f debugLocation;
	
	string name;
	string currentPortrait;
	ofRectangle rect;
	ofImage mask;
	int currentCameraSample;
	
	//portrait name -> camera list
	map<string, vector<ofNode> > cameraPositions;
	ofxGameCamera cam;
	ofCamera normalCam;
	
	ofxFloatSlider brightness;
	ofxFloatSlider contrast;

	
	ofCamera& getCameraRef();


	bool automode;
	
	bool cameraTransition;
	float transitionStart;
	float transitionEnd;;
	ofNode beginning;
	ofNode target;
	void updateCameraSwoop();
	
	void setup();
	void update();
	
	void save();
	void load();
	
	void sampleCamera();
	void nextPose();
	void deleteCurrentPose();
	
	void updateCameraPose();
	
	float nextChangeTime;

	//set externally
	float minChangeTime;
	float changeTimeVariance;
	
	//custom geometry
	ofxFloatSlider *maxExtend;
	ofxFloatSlider *varianceEffect;
	
	
};