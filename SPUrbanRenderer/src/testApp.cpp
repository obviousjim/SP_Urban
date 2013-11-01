/**
 * SP_Urban renderer
 * Display for the SP_Urban facade, Spectacle of Change portrait series
 *
 * (C) James George 2013
 *
 */

#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
    
    ofSetFrameRate(60);
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

	gui.add(headSphereRadius.setup("head radius", ofParameter<float>(), 0, 300));
	gui.add(headEffectFalloff.setup("head falloff", ofParameter<float>(), 1, 1000));
	gui.add(maxExtend.setup("max geom extend", ofParameter<float>(), .5, 1.0));
	gui.add(extendThreshold.setup("geom extend thresh", ofParameter<float>(), 0, 300));
	gui.add(extendFalloff.setup("geom extend falloff", ofParameter<float>(), 20, 500));

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

	fbo.allocate(1024, 768, GL_RGB, 4);
	
	highlightScreen = NULL;
	
	currentPortraitIndex = 0;
	switchPortrait();

	loadHeadPositions();

	generateGeometry();
//	ofToggleFullscreen();
	
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

void testApp::saveHeadPositions(){
	map<string, ofVec2f>::iterator it;
	ofxXmlSettings headPosSave;
	headPosSave.addTag("heads");
	headPosSave.pushTag("heads");
	int h = 0;
	for(it = headPositions.begin(); it != headPositions.end(); it++){
		headPosSave.addTag("head");
		headPosSave.addAttribute("head","name", it->first, h);
		headPosSave.pushTag("head",h);
		headPosSave.addValue("x", it->second.x);
		headPosSave.addValue("y", it->second.y);
		headPosSave.popTag();
		h++;
	}
	headPosSave.popTag();//heads;
	headPosSave.save("HeadPositions.xml");
}

void testApp::loadHeadPositions(){
	ofxXmlSettings headPosSave;
	if(!headPosSave.load("HeadPositions.xml")){
		ofLogError("testApp::loadHeadPositions") << "Couldn't load head positions";
		return;
	}
	headPositions.clear();
	headPosSave.pushTag("heads");
	int numheads = headPosSave.getNumTags("head");
	for(int i = 0; i < numheads; i++){
		string headName = headPosSave.getAttribute("head", "name", "", i);
		
		if(headName == "") continue;
		
		headPosSave.pushTag("head",i);
		headPositions[headName].x = headPosSave.getValue("x", 0);
		headPositions[headName].y = headPosSave.getValue("y", 0);
		headPosSave.popTag();//head;
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
			else {
				
				for(int x = c * colStep; x < (c+1) * colStep ; x += vertStep ) {
					
					//add  two triangles for each
					ofIndexType startIndex = mesh.getNumIndices();
					mesh.addIndex(startIndex+0);
					mesh.addIndex(startIndex+1);
					mesh.addIndex(startIndex+2);

					mesh.addIndex(startIndex+3);
					mesh.addIndex(startIndex+4);
					mesh.addIndex(startIndex+5);

					ofVec2f a = ofVec2f(x,y);
					ofVec2f b = ofVec2f(x+vertStep,y);
					ofVec2f c = ofVec2f(x+vertStep,y+vertStep);
					ofVec2f d = ofVec2f(x,y+vertStep);
					
					float av = ofRandom(.8,1.0);
					float bv = ofRandom(.8,1.0);
					float cv = ofRandom(.8,1.0);
					float dv = ofRandom(.8,1.0);
					
					ofVec2f center = (a+b+c+d)*.25;
					
					ofVec3f na = a-center;
					ofVec3f nb = b-center;
					ofVec3f nc = c-center;
					ofVec3f nd = d-center;

					na.z = av;
					nb.z = bv;
					nc.z = cv;
					nd.z = dv;
					
					ofVec3f colvec = ofVec3f(col.r,col.g,col.b);
					//vertices are actually colors here
					mesh.addVertex(colvec);
					mesh.addVertex(colvec);
					mesh.addVertex(colvec);
					
					mesh.addVertex(colvec);
					mesh.addVertex(colvec);
					mesh.addVertex(colvec);
					
					//Tex coords are actually vertices here
					mesh.addTexCoord(a);
					mesh.addTexCoord(b);
					mesh.addTexCoord(d);
					
					mesh.addTexCoord(b);
					mesh.addTexCoord(c);
					mesh.addTexCoord(d);
					
					//colors are the neighbor normal locations
					mesh.addColor(ofFloatColor(nb.x,nb.y,nd.x,nd.y)); //neighbors of A1
					mesh.addColor(ofFloatColor(na.x,na.y,nd.x,nd.y)); //neighbors of B1
					mesh.addColor(ofFloatColor(na.x,na.y,nb.x,nb.y)); //neighbors of D1

					mesh.addColor(ofFloatColor(nc.x,nc.y,nd.x,nd.y)); //neighbors of B2
					mesh.addColor(ofFloatColor(nb.x,nb.y,nd.x,nd.y)); //neighbors of C2
					mesh.addColor(ofFloatColor(nb.x,nb.y,nc.x,nc.y)); //neighbors of D2
					
					//normals point to the center point
					mesh.addNormal(na);
					mesh.addNormal(nb);
					mesh.addNormal(nd);
					
					mesh.addNormal(nb);
					mesh.addNormal(nc);
					mesh.addNormal(nd);
				}
				
				skipping = ofRandomuf() > .95;
			}
		}
	}

	//create variance image
	varianceImage.allocate(640, 480, OF_IMAGE_GRAYSCALE);
	for(int i = 0; i < varianceImage.getWidth()*varianceImage.getHeight(); i++){
		if(ofRandomuf() > .90){
			varianceImage.getPixels()[i] = 0; //dead pixel
		}
		else{
			varianceImage.getPixels()[i] = ofMap(powf(ofRandomuf(), 4.), 1.0, 0.0, .9, 1.0);
		}
	}
	varianceImage.update();
	
	speedVarianceImage.allocate(640, 1, OF_IMAGE_GRAYSCALE);
	for (int i = 0; i < speedVarianceImage.getWidth(); i++) {
		speedVarianceImage.getPixels()[i] = ofMap(powf(ofRandomuf(),2.),1.0, 0.0, .3, 1.0);
	}
	speedVarianceImage.update();
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
		ofSystemAlertDialog("Couldn't load path " + paths[currentPortraitIndex] );
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
        
		fbo.begin();
		ofClear(1.,.1,.1,1);
		
		//BACKDROP
		ofSetColor(ofColor::black);
		ofDisableAlphaBlending();
		for(int i = 0; i < screens.size(); i++){
			ofRect(screens[i]->rect);
		}
		
		//PORTRAIT
		ofEnableBlendMode(OF_BLENDMODE_ADD);
		ofSetColor(ofColor::white);
		glDisable(GL_DEPTH_TEST);
		for(int i = 0; i < screens.size(); i++){
			screens[i]->getCameraRef().begin(ofRectangle(screens[i]->rect.x,
														 fbo.getHeight() - screens[i]->rect.y - screens[i]->rect.height,
														 screens[i]->rect.width,
														 screens[i]->rect.height));
			renderer.bindRenderer();
			
			renderer.getShader().setUniform1f("flowPosition", flowSpeed * ofGetElapsedTimef());
			renderer.getShader().setUniform1f("brightness", screens[i]->brightness);
			renderer.getShader().setUniform1f("contrast", screens[i]->contrast);
			renderer.getShader().setUniform2f("headPosition",
											  headPositions[player.getScene().name].x,
											  headPositions[player.getScene().name].y);
			
			renderer.getShader().setUniform1f("headSphereRadius",headSphereRadius);
			renderer.getShader().setUniform1f("headEffectFalloff",headEffectFalloff);

			renderer.getShader().setUniform1f("maxExtend",maxExtend);
			renderer.getShader().setUniform1f("extendThreshold",extendThreshold);
			renderer.getShader().setUniform1f("extendFalloff",extendFalloff);
			
			renderer.getShader().setUniformTexture("varianceTex",varianceImage, 3);
			renderer.getShader().setUniformTexture("speedVarianceTex",speedVarianceImage, 4);

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
		}

		fbo.end();
		
		syphonServer.publishTexture(&fbo.getTextureReference());
		
		fbo.draw(0, 0);
    }

	ofDrawBitmapString("name " + player.getScene().name +
					   "\nfps " + ofToString(ofGetFrameRate()) +
					   "\ntime til next " + ofToString(nextPortraitTime - ofGetElapsedTimef(),2),
					   led1.rect.getMaxX(), led1.rect.y);
	
	ofEnableAlphaBlending();

	for(int i = 0; i < screens.size(); i++){
		ofPushStyle();
		if(highlightScreen == screens[i] ){
			ofNoFill();
			ofSetColor(255, 100, 0);
			ofRect(screens[i]->rect);
		}
		else if(!screens[i]->automode){
			ofNoFill();
			ofSetColor(100, 255, 30);
			ofRect(screens[i]->rect);
		}
		ofPopStyle();
		
		screens[i]->drawDebug();
	}


	renderer.getDepthTexture().draw(depthRect);
	
	ofPushStyle();
	ofSetColor(255, 100, 5, 150);
	ofNoFill();
	ofCircle( headPositions[player.getScene().name] * .25 + depthRect.getMin(), 7);
	ofPopStyle();
	
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
	if(key == OF_KEY_RIGHT){
		gotoNextPortrait();
	}
}

//--------------------------------------------------------------
void testApp::exit(){
	for(int i = 0; i < screens.size(); i++){
		screens[i]->saveSettings();
	}
	saveHeadPositions();
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
	ofVec2f m(x,y);
	highlightScreen = NULL;
	for(int i = 0; i < screens.size(); i++){
		if(screens[i]->rect.inside(m)){
			highlightScreen = screens[i];
			break;
		}
	}

	if(depthRect.inside(m)){
		headPositions[player.getScene().name] = (m - depthRect.getMin()) * 4;
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
}
