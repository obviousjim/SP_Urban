//
//  PortraitScreen.cpp
//  example-simpleRenderer
//
//  Created by James George on 10/27/13.
//
//

#include "PortraitScreen.h"
#include "ofxXmlSettings.h"

PortraitScreen::PortraitScreen(){
	automode = true;
	minChangeTime = 5;
	changeTimeVariance = 2;
	nextChangeTime = 0;
	currentCameraSample = -1;
	pair = NULL; //used to ignore duplicates
}

void PortraitScreen::setup(){
	
	cam.setup();
	cam.speed = 20;
	cam.cameraPositionFile = "_tempsave_" + name + ".xml";
	cam.autosavePosition = true;
	cam.targetNode.setPosition(ofVec3f());
	cam.targetNode.setOrientation(ofQuaternion());
	cam.targetXRot = -180;
	cam.targetYRot = 0;
	cam.rotationZ = 0;
	cam.loadCameraPosition();

	load();
}

void PortraitScreen::save(){
	map<string, vector<ofNode> >::iterator it;
	ofxXmlSettings cameraSave;
	cameraSave.addTag("poses");
	cameraSave.pushTag("poses");
	int p = 0;
	for(it = cameraPositions.begin(); it != cameraPositions.end(); it++){
		cameraSave.addTag("portrait");
		cameraSave.addAttribute("portrait", "name", it->first, p);
		cameraSave.pushTag("portrait", p);

		vector<ofNode>& poses = it->second;
		for(int pose = 0; pose < poses.size(); pose++){
			cameraSave.addTag("pose");
			cameraSave.pushTag("pose", pose);
			ofNode& n = poses[pose];
			cameraSave.addTag("position");
			cameraSave.pushTag("position");
			cameraSave.addValue("X", n.getPosition().x);
			cameraSave.addValue("Y", n.getPosition().y);
			cameraSave.addValue("Z", n.getPosition().z);
			cameraSave.popTag(); //pop position
			
			cameraSave.addTag("rotation");
			cameraSave.pushTag("rotation");
			cameraSave.addValue( "X", n.getOrientationQuat().x() );
			cameraSave.addValue( "Y", n.getOrientationQuat().y() );
			cameraSave.addValue( "Z", n.getOrientationQuat().z() );
			cameraSave.addValue( "W", n.getOrientationQuat().w() );
			cameraSave.popTag();//rotation
			
			cameraSave.popTag(); //pose
		}
		cameraSave.popTag();//portait;
		p++;
	}
	cameraSave.save("cameraposes_" + name + ".xml");
}

ofCamera& PortraitScreen::getCameraRef(){
	return automode ? normalCam : cam;
}
void PortraitScreen::load(){
	ofxXmlSettings cameraSave;
	string path = "cameraposes_" + name + ".xml";
	if(!cameraSave.load(path)){
		ofLogError("PortraitScreen::load") << "Couldn't load " << path;
		return;
	}

	cameraPositions.clear();
	cameraSave.pushTag("poses");
	int numPortraits = cameraSave.getNumTags("portrait");
	
	for(int p = 0; p < numPortraits; p++){
		string name = cameraSave.getAttribute("portrait", "name", "", p);
		cameraSave.pushTag("portrait", p);
		int numPoses = cameraSave.getNumTags("pose");
		cout << "LOADING " << numPoses << " poses for " << name << endl;
		for(int pose = 0; pose < numPoses; pose++){
			cameraSave.pushTag("pose",pose);

			ofNode n;
			cameraSave.pushTag("position");
			n.setPosition(ofVec3f(
				cameraSave.getValue("X", 0),
				cameraSave.getValue("Y", 0),
				cameraSave.getValue("Z", 0)));
			cameraSave.popTag(); //pop position
			

			cameraSave.pushTag("rotation");

			n.setOrientation(ofQuaternion(
				cameraSave.getValue("X", 0.),
				cameraSave.getValue("Y", 0.),
				cameraSave.getValue("Z", 0.),
				cameraSave.getValue("W", 1.)));
			
			cameraSave.popTag();//rotation
			
			cameraPositions[name].push_back(n);

			cameraSave.popTag(); //pose;
		}
		cameraSave.popTag();//portait;
		p++;
	}
}

void PortraitScreen::sampleCamera(){
	ofNode n;
	n.setPosition(cam.getPosition());
	n.setOrientation(cam.getOrientationQuat());
	cameraPositions[currentPortrait].push_back(n);

	save();
}

void PortraitScreen::deleteCurrentPose(){
	if(currentCameraSample >= 0 && currentCameraSample < cameraPositions[currentPortrait].size()){
		cameraPositions[currentPortrait].erase(cameraPositions[currentPortrait].begin() + currentCameraSample);
		nextChangeTime = ofGetElapsedTimef();
		save();
	}

}

void PortraitScreen::nextPose(){
	if(cameraPositions[currentPortrait].size() > 0){
		currentCameraSample = (currentCameraSample + 1) % cameraPositions[currentPortrait].size();
		updateCameraPose();
	}
}

void PortraitScreen::update(){
	if(automode){
		if(nextChangeTime < ofGetElapsedTimef()) {
			nextChangeTime = ofGetElapsedTimef() + minChangeTime + ofRandom(changeTimeVariance);
			vector<ofNode>& poses = cameraPositions[currentPortrait];
			if(poses.size() == 0){
				ofLogError("PortraitScreen::update") << "No camera positions for " << currentPortrait << " on screen " << name;
				return;
			}
			
			int tries = 10;
			do {
				currentCameraSample = ofRandom(poses.size());
			} while(currentCameraSample != currentCameraSample && tries-- > 0);
					
			updateCameraPose();
		}
	}
}

void PortraitScreen::updateCameraPose(){
	vector<ofNode>& poses = cameraPositions[currentPortrait];	
	normalCam.setPosition( poses[currentCameraSample].getPosition() );
	normalCam.setOrientation( poses[currentCameraSample].getOrientationQuat() );
}

void PortraitScreen::draw(){
	if(mask.isAllocated()){
		mask.draw(rect);
	}
}

void PortraitScreen::drawDebug(){
	char debug[1024];
	sprintf(debug, "Name %s \nAutoMode? %s \nTime left %.02f\nCurrent Pose %d \nTotal Poses %ld",
			name.c_str(), (automode ? "YES" : "NO"),
			nextChangeTime-ofGetElapsedTimef(),
			currentCameraSample,
			cameraPositions[currentPortrait].size());
	
	ofDrawBitmapString(debug, debugLocation);
}