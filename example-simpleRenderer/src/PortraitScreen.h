
#pragma once

#include "ofMain.h"
#include "ofxGameCamera.h"

class PortraitScreen {
  public:
	
	PortraitScreen();
	
	string name;
	ofRectangle rect;
	ofImage mask;
	vector<ofNode> cameraPositions;
	ofxGameCamera cam;
	//ofEasyCam cam;
	bool automode;
	
	void setup();
	void update();
	void draw();
	
	void save();
	void load();
	
	void sampleCamera();
	
};