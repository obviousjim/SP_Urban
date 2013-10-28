/**
 * Example - Mesh Builder
 * This example shows how to create a RGBD Mesh on the CPU
 *
 *
 * James George 2012 
 * Released under the MIT License
 *
 * The RGBDToolkit has been developed with support from the STUDIO for Creative Inquiry and Eyebeam
 */

#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
    
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    ofBackground(25);
	
    nextPortraitTime = 0;
	
    //set up the game camera
    
    xrotate = 0;
    yrotate = 0;
    

	renderer.setShaderPath("shaders/rgbdspurban");
	

	led1.name = "LED1";
	led1.rect = ofRectangle(280, 10, 760-280,298-10);
	led1.debugLocation.x = led1.rect.getMaxX();
	led1.debugLocation.y = led1.rect.getMaxY()-70;
	
	led2.name = "LED2";
	led2.rect = ofRectangle(280, 310, 760-280,598-310);
	led2.debugLocation.x = led2.rect.getMaxX();
	led2.debugLocation.y = led2.rect.getY();
	
	leftFacade.name = "LEFT_FACADE";
	leftFacade.rect = ofRectangle(36, 258,98-36,426-258);
	leftFacade.mask.loadImage("facade/leftfacade.png");
	leftFacade.debugLocation.x = leftFacade.rect.getX()-30;
	leftFacade.debugLocation.y = leftFacade.rect.getMaxY()+15;
	
	centerFacade.name = "CENTER_FACADE";
	centerFacade.rect = ofRectangle(98, 258,192-98,426-258);
	centerFacade.mask.loadImage("facade/centerfacade.png");
	centerFacade.debugLocation.x = centerFacade.rect.getX();
	centerFacade.debugLocation.y = centerFacade.rect.getY()-60;
	
	rightFacade.name = "RIGHT_FACADE";
	rightFacade.rect = ofRectangle(192, 258,251-192,426-258);
	rightFacade.mask.loadImage("facade/rightfacade.png");
	rightFacade.debugLocation.x = rightFacade.rect.getX()-50;
	rightFacade.debugLocation.y = rightFacade.rect.getMaxY()+15;

	
	screens.push_back(&led1);
	screens.push_back(&led2);
	screens.push_back(&leftFacade);
	screens.push_back(&centerFacade);
	screens.push_back(&rightFacade);

	gui.setup("tests");
    
	gui.add(xrotate.setup("xrotate", ofParameter<float>(), -4., 4.));
    gui.add(yrotate.setup("yrotate", ofParameter<float>(), -4., 4.));
	gui.add(zclip.setup("zclip",ofParameter<float>(), 500, 2000));
	gui.add(timePerPortrait.setup("time per portrait", ofParameter<float>(), 10, 60));
	gui.add(flowSpeed.setup("flow", ofParameter<float>(), 0, 200));

	for(int i = 0; i < screens.size(); i++){
		gui.add( screens[i]->brightness.setup(screens[i]->name + " bri",ofParameter<float>(), 0, 2) );
		gui.add( screens[i]->contrast.setup(screens[i]->name + " con",ofParameter<float>(), 0, 3) );
		screens[i]->setup();
	}
	
	gui.loadFromFile("defaultSettings.xml");
	gui.setPosition(ofPoint(led2.rect.getMaxX() + 10, led2.rect.getMinY() + 75));
					
	ofxXmlSettings pathxml;
	pathxml.load("paths.xml");
	pathxml.pushTag("paths");
	int numpaths = pathxml.getNumTags("path");
	for(int i = 0; i < numpaths; i++){
		paths.push_back(pathxml.getValue("path", "", i));
	}
	pathxml.popTag();
	
	depthRect = ofRectangle(10 ,10,160,120);
	
//	if(leftmask.getWidth() != leftFacade.getWidth() || leftmask.getHeight() != leftFacade.getHeight()){
//		ofLogError("setup") << "left mask dimensions do not match!" << leftmask.getWidth() << " " << leftFacade.getWidth() << " " << leftmask.getHeight() << " " << leftFacade.getHeight();
//	}
//	if(centermask.getWidth() != centerFacade.getWidth() || centermask.getHeight() != centerFacade.getHeight()){
//		ofLogError("setup") << "center mask dimensions do not match!" << centermask.getWidth() << " " << centerFacade.getWidth() << " " << centermask.getHeight() << " " << centerFacade.getHeight();
//	}
//	if(rightmask.getWidth() != rightFacade.getWidth() || rightmask.getHeight() != rightFacade.getHeight()){
//		ofLogError("setup") << "center mask dimensions do not match!" << rightmask.getWidth() << " " << rightFacade.getWidth() << " " << rightmask.getHeight() << " " << rightFacade.getHeight();
//	}

	highlightScreen = NULL;
	
	currentPortraitIndex = 0;
	switchPortrait();
	//attemping to load the last scene
//    loadDefaultScene();
	
	generateGeometry();
}

//--------------------------------------------------------------
bool testApp::loadNewScene(){
    ofFileDialogResult r = ofSystemLoadDialog("Select a Scene", true);
    if(r.bSuccess){
        return loadScene(r.getPath());
    }
    return false;
}

//--------------------------------------------------------------
bool testApp::loadDefaultScene(){
    ofxXmlSettings settings;
    if(settings.loadFile("RGBDSimpleSceneDefaults.xml")){
        if(!loadScene(settings.getValue("defaultScene", ""))){
            return loadNewScene();
        }
        return true;
    }
    return loadNewScene();
}

//--------------------------------------------------------------
bool testApp::loadScene(string takeDirectory){
    if(player.setup(takeDirectory)){
        ofxXmlSettings settings;
        settings.loadFile("RGBDSimpleSceneDefaults.xml");
        settings.setValue("defaultScene", player.getScene().mediaFolder);
        settings.saveFile();
        renderer.setup(player.getScene().calibrationFolder);
        
        //populate
        player.getVideoPlayer()->setPosition(.5);
        player.update();
		
		renderer.setRGBTexture(*player.getVideoPlayer());
		renderer.setDepthImage(player.getDepthPixels());
        
        return true;
    }
    return false;
}

//--------------------------------------------------------------
void testApp::update(){
	
//    if(loadNew){
//        loadNewScene();
//    }
//    
    //copy any GUI changes into the mesh
    renderer.colorMatrixRotate.x = xrotate;
	renderer.colorMatrixRotate.y = yrotate;
	renderer.farClip = zclip;

    //update the mesh if there is a new depth frame in the player
    player.update();
    if(player.isFrameNew()){
        renderer.update();
    }
	
	if(ofGetElapsedTimef() > nextPortraitTime){
		bool hasComposeMode = false;
		for(int i = 0; i < screens.size(); i++){
			hasComposeMode |= !screens[i]->automode;
		}
		
		if(!hasComposeMode){
			gotoNextPortrait();
			
			nextPortraitTime = ofGetElapsedTimef() + timePerPortrait;
			for(int i = 0; i < screens.size(); i++){
				screens[i]->nextPose();
			}
		}
	}

	for(int i = 0; i < screens.size(); i++){
		screens[i]->currentPortrait = player.getScene().name;
		screens[i]->update();
		if(screens[i] == highlightScreen){
			screens[i]->cam.applyRotation = screens[i]->cam.applyTranslation = true;
		}
		else{
			screens[i]->cam.applyRotation = screens[i]->cam.applyTranslation = false;
		}
	}
}


void testApp::generateGeometry(){
	
	mesh.clear();

	int columns = 40;
	int vertsPerColumn = 3;
	float colStep = 640. / columns;
	float vertStep = colStep / (vertsPerColumn+1);
	bool skipping = false;
	
	cout << "creating mesh with colstep " << colStep << " and vert step " << vertStep << endl;

	for(int c = 0; c < columns; c++){
		//draw one column
		
		ofFloatColor col;
		col = ofFloatColor::fromHsb(ofRandomuf(), 1.0, 1.0);		
		for(int y = 0; y < 480; y += vertStep){
			if(skipping){
				skipping = ofRandomuf() > .7;
				col = ofFloatColor::fromHsb(ofRandomuf(), 1.0, 1.0);
			}
			else{				
				for(int x = c * colStep; x < (c+1) * colStep - vertStep; x += vertStep ) {
					
					//add  two triangles for each
					ofIndexType startIndex = mesh.getNumIndices();
					mesh.addIndex(startIndex+0);
					mesh.addIndex(startIndex+1);
					mesh.addIndex(startIndex+2);

					mesh.addIndex(startIndex+3);
					mesh.addIndex(startIndex+4);
					mesh.addIndex(startIndex+5);
									
					ofVec3f a = ofVec3f(x,y,0);
					ofVec3f b = ofVec3f(x+vertStep,y,0);
					ofVec3f c = ofVec3f(x+vertStep,y+vertStep,0);
					ofVec3f d = ofVec3f(x,y+vertStep,0);
					
					mesh.addVertex(a);
					mesh.addVertex(b);
					mesh.addVertex(d);

					mesh.addVertex(b);
					mesh.addVertex(c);
					mesh.addVertex(d);
					
					//TODO: colors!!!
//					ofFloatColor color(ofRandomuf(),ofRandomuf(),ofRandomuf());
//					mesh.addColor(col);
//					mesh.addColor(col);
//					mesh.addColor(col);
//					
//					mesh.addColor(col);
//					mesh.addColor(col);
//					mesh.addColor(col);
				}
				
				skipping = ofRandomuf() > .95;				
			}
		}
	}

	renderer.setSimplification( ofVec2f(vertStep,vertStep) );
	
}

//--------------------------------------------------------------
void testApp::gotoNextPortrait(){
	currentPortraitIndex = (currentPortraitIndex + 1) % paths.size();
	switchPortrait();	
}

//--------------------------------------------------------------
void testApp::gotoPreviousPortrait(){
	currentPortraitIndex = (paths.size() + currentPortraitIndex - 1) % paths.size();
	switchPortrait();
}

//--------------------------------------------------------------
void testApp::switchPortrait(){
	if(!loadScene( paths[currentPortraitIndex] )){
		ofLogError("testApp::switchPortrait") << "Failed to load portrait " << paths[currentPortraitIndex];
		return;
	}
	
	player.getVideoPlayer()->play();
	player.getVideoPlayer()->setSpeed(.5);
	player.getVideoPlayer()->setVolume(0.);
	
}

//--------------------------------------------------------------
void testApp::draw(){
    if(player.isLoaded()){
        
		//BACKDROP
		ofSetColor(ofColor::black);
		ofDisableAlphaBlending();
		for(int i = 0; i < screens.size(); i++){
			ofRect(screens[i]->rect);
		}
		
		//PORTRAIT
		ofEnableBlendMode(OF_BLENDMODE_SCREEN);
		ofSetColor(ofColor::white);
		glDisable(GL_DEPTH_TEST);
		for(int i = 0; i < screens.size(); i++){
			screens[i]->getCameraRef().begin(screens[i]->rect);
			renderer.bindRenderer();
			
			renderer.getShader().setUniform1f("flowPosition", flowSpeed * ofGetElapsedTimef());
			renderer.getShader().setUniform1f("brightness", screens[i]->brightness);
			renderer.getShader().setUniform1f("contrast", screens[i]->contrast);
			
			mesh.draw();
			renderer.unbindRenderer();
			screens[i]->getCameraRef().end();
		}


		//MASK
		ofEnableAlphaBlending();
		for(int i = 0; i < screens.size(); i++){
			if(screens[i]->mask.isAllocated()){
				screens[i]->mask.draw(screens[i]->rect);
			}
			screens[i]->drawDebug();
		}

		ofPushStyle();
		ofEnableAlphaBlending();
		if(highlightScreen != NULL){
			ofNoFill();
			ofSetColor(255, 100, 0);
			ofRect(highlightScreen->rect);
		}
		ofPopStyle();
    }

	ofDrawBitmapString("name " + player.getScene().name +
					   "\nfps " + ofToString(ofGetFrameRate()) +
					   "\ntime til next " + ofToString(nextPortraitTime - ofGetElapsedTimef(),2),
					   led1.rect.getMaxX(), led1.rect.y);
	

	renderer.getDepthTexture().draw(depthRect);
	for(int i = 0; i < screens.size(); i++){
//		screens[i]->controls.draw();
	}
    gui.draw();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	
    if(key == ' '){
        player.togglePlay();
    }
	if(key == 'R'){
		renderer.reloadShader();
	}
	if(key == 'C' && highlightScreen != NULL){
		highlightScreen->automode = !highlightScreen->automode;
	}
	
	if(key == 'P' && highlightScreen != NULL){
		highlightScreen->sampleCamera();
	}
	if(key == 'D' && highlightScreen != NULL){
		highlightScreen->deleteCurrentPose();
	}
	if(key == 'N' && highlightScreen != NULL){
		highlightScreen->nextPose();
	}
}

//--------------------------------------------------------------
void testApp::exit(){
	for(int i = 0; i < screens.size(); i++){
		screens[i]->saveSettings();
	}
    gui.saveToFile("defaultSettings.xml");
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	highlightScreen = NULL;
	for(int i = 0; i < screens.size(); i++){
		if(screens[i]->rect.inside(x, y)){
			highlightScreen = screens[i];
			break;
		}
	}

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){
//	ofDirectory dir(dragInfo.files[0]);
//	if( dir.isDirectory() && ofxRGBDScene::isFolderValid(dragInfo.files[0]) ){
//		loadScene(dragInfo.files[0]);
//	}
}
