
#pragma once

#include "ofMain.h"
#include "ofxGameCamera.h"

class PortraitScreen {
  public:
	
	PortraitScreen();
	
	PortraitScreen* pair;
	
	void drawDebug();
	
	string name;
	string currentPortrait;
	ofRectangle rect;
	ofImage mask;
	int currentCameraSample;
	
	//portrait name -> camera list
	map<string, vector<ofNode> > cameraPositions;
	ofxGameCamera cam;
	ofCamera normalCam;
	
	ofCamera& getCameraRef();
	
	//ofEasyCam cam;
	bool automode;
	
	void setup();
	void update();
	void draw();
	
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
	
};