//
//  PortraitScreen.cpp
//  example-simpleRenderer
//
//  Created by James George on 10/27/13.
//
//

#include "PortraitScreen.h"


PortraitScreen::PortraitScreen(){
	automode = false;
}

void PortraitScreen::save(){
	
}

void PortraitScreen::load(){
	
}

void PortraitScreen::sampleCamera(){
	cameraPositions.push_back(cam);
}

void PortraitScreen::setup(){
	
	cam.setup();
	cam.speed = 20;
	cam.cameraPositionFile = name + "_campos.xml";
	cam.autosavePosition = true;
	cam.targetNode.setPosition(ofVec3f());
	cam.targetNode.setOrientation(ofQuaternion());
	cam.targetXRot = -180;
	cam.targetYRot = 0;
	cam.rotationZ = 0;
}

void PortraitScreen::update(){
	
}

void PortraitScreen::draw(){
	if(mask.isAllocated()){
		mask.draw(rect);
	}
}