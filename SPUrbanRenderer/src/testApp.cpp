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
    
	srand(ofGetSeconds());
	cout << "first randoms " << ofRandomuf() << " " << ofRandomuf() << " " << ofRandomuf() << endl;
    ofSetFrameRate(60);
    ofBackground(25);
	
    nextPortraitTime = 0;
	currentPalette = 0;
	pureColorFlickerPos = 0;
	hasSubtitles = false;
	easterEggPlayed = false;
	soundPlaying = true;
	playing = false;
	shouldStartPlaying = false;
	shouldStopPlaying = false;
	playFullScreen = false;
	hasSound = false;
	
    //set up the game camera
    xrotate = 0;
    yrotate = 0;
    
	renderer.setShaderPath("shaders/rgbdspurban");
	
	led1.name = "LED1";
	led1.rect = ofRectangle(280, 10, 760-280,298-10);
	led1.debugLocation.x = led1.rect.getMaxX();
	led1.debugLocation.y = led1.rect.getMaxY()-70;
	led1.maxExtend = &maxExtendLED;
	led1.varianceEffect = &varianceEffectLED;

	led2.name = "LED2";
	led2.rect = ofRectangle(280, 310, 760-280,598-310);
	led2.debugLocation.x = led2.rect.getMaxX();
	led2.debugLocation.y = led2.rect.getY();
	led2.maxExtend = &maxExtendLED;
	led2.varianceEffect = &varianceEffectLED;
	
	leftFacade.name = "LEFT_FACADE";
	leftFacade.rect = ofRectangle(36, 258,98-36,426-258);
	leftFacade.mask.loadImage("facade/leftfacade.png");
	leftFacade.debugLocation.x = leftFacade.rect.getX()-30;
	leftFacade.debugLocation.y = leftFacade.rect.getMaxY()+15;
	leftFacade.maxExtend = &maxExtendFacade;
	leftFacade.varianceEffect = &varianceEffectFacade;
	
	centerFacade.name = "CENTER_FACADE";
	centerFacade.rect = ofRectangle(98, 258,192-98,426-258);
	centerFacade.mask.loadImage("facade/centerfacade.png");
	centerFacade.debugLocation.x = centerFacade.rect.getX();
	centerFacade.debugLocation.y = centerFacade.rect.getY()-60;
	centerFacade.maxExtend = &maxExtendFacade;
	centerFacade.varianceEffect = &varianceEffectFacade;
	
	rightFacade.name = "RIGHT_FACADE";
	rightFacade.rect = ofRectangle(192, 258,251-192,426-258);
	rightFacade.mask.loadImage("facade/rightfacade.png");
	rightFacade.debugLocation.x = rightFacade.rect.getX()-50;
	rightFacade.debugLocation.y = rightFacade.rect.getMaxY()+15;
	rightFacade.maxExtend = &maxExtendFacade;
	rightFacade.varianceEffect = &varianceEffectFacade;
	
	screens.push_back(&led1); // 0
	screens.push_back(&led2); // 1
	screens.push_back(&leftFacade);		// 2
	screens.push_back(&centerFacade);	// 3
	screens.push_back(&rightFacade);	// 4

	gui.setup("tests");
    
	gui.add(xrotate.setup("xrotate", ofParameter<float>(), -4., 4.));
    gui.add(yrotate.setup("yrotate", ofParameter<float>(), -4., 4.));
	gui.add(zclip.setup("zclip",ofParameter<float>(), 500, 2000));
	gui.add(timePerPortrait.setup("time per portrait", ofParameter<float>(), 10, 60));
	gui.add(flowSpeed.setup("flow", ofParameter<float>(), 0, 200));

	gui.add(headSphereRadius.setup("head radius", ofParameter<float>(), 0, 300));
	gui.add(headEffectFalloff.setup("head falloff", ofParameter<float>(), 1, 1000));
	gui.add(extendThreshold.setup("geom extend thresh", ofParameter<float>(), 0, 300));
	gui.add(extendFalloff.setup("geom extend falloff", ofParameter<float>(), 20, 500));
	gui.add(maxExtendFacade.setup("max extend Facade", ofParameter<float>(), .5, 1.0));
	gui.add(maxExtendLED.setup("max extend LED", ofParameter<float>(), .5, 1.0));
	gui.add(varianceEffectFacade.setup("variance Facade", ofParameter<float>(), 0, 1.0));
	gui.add(varianceEffectLED.setup("variance LED", ofParameter<float>(), 0, 1.0));
	gui.add(pureColorThreshold.setup("pure color thresh", ofParameter<float>(), .2, 1.0));
	gui.add(pureColorFlicker.setup("pure color flicker", ofParameter<float>(), .01, .1));
	
	gui.add(facadeColorCorrectR.setup("correct r", ofParameter<float>(), -.2,.2));
	gui.add(facadeColorCorrectB.setup("correct g", ofParameter<float>(), -.2,.2));
	gui.add(facadeColorCorrectG.setup("correct b", ofParameter<float>(), -.2,.2));

	for(int i = 0; i < screens.size(); i++){
		gui.add( screens[i]->brightness.setup(screens[i]->name + " bri",ofParameter<float>(), 0, 2) );
		gui.add( screens[i]->contrast.setup(screens[i]->name + " con",ofParameter<float>(), 0, 3) );
		screens[i]->setup();
	}
	
	gui.loadFromFile("defaultSettings.xml");
	gui.setPosition(ofPoint(led2.rect.getMaxX() + 10,
							led2.rect.getMinY() + 75));
					
	ofxXmlSettings pathxml;
	pathxml.load("paths.xml");
	pathxml.pushTag("paths");
	int numpaths = pathxml.getNumTags("path");
	for(int i = 0; i < numpaths; i++){
		paths.push_back(pathxml.getValue("path", "", i));
	}
	pathxml.popTag();
	
	depthRect = ofRectangle(10,10,160,120);

	fbo.allocate(1024, 768, GL_RGB, 4);
	
	highlightScreen = NULL;
	
	titles.setup("subtitles/AxisStd-Regular.otf", 9);
	
	currentPortraitIndex = 0;

	loadHeadPositions();
	loadPalettes();
	generateGeometry();
	
//	switchPortrait();
//	music.loadSound("music/MASS.mp3");
//	music.setLoop(true);
//	music.play();
//	music.setVolume(.65);

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
	
	if(shouldStartPlaying){
		switchPortrait();
		music.loadSound("music/MASS.mp3");
		music.setLoop(true);
		music.play();
		music.setVolume(.65);
		shouldStartPlaying = false;
		playing = true;
	}
	
	if(!playing) return;
	
    //copy any GUI changes into the mesh
    renderer.colorMatrixRotate.x = xrotate;
	renderer.colorMatrixRotate.y = yrotate;
	renderer.farClip = zclip;
	pureColorFlickerPos += powf(pureColorFlicker,2.);
	
    //update the mesh if there is a new depth frame in the player
    player.update();
    if(player.isFrameNew()){
        renderer.update();
    }
	
	if(hasSound){
		sound.update();
		if(hasSubtitles){
			titles.setTimeInSeconds(sound.getPosition()*sound.getDuration());
//			cout << "setting titles " << endl;
//			if(titles.getCurrentUnit() != NULL) cout << "Current title is " << titles.getCurrentUnit()->getLines()[0] << endl;
		}
	}
	
	hasComposeMode = false;
	for(int i = 0; i < screens.size(); i++){
		hasComposeMode |= !screens[i]->automode;
	}
	
	if( (!hasSound && ofGetElapsedTimef() > nextPortraitTime) || (hasSound && !sound.isPlaying()) ){

		if(!hasComposeMode){
			gotoNextPortrait();
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

void testApp::loadPalettes(){
	ofDirectory dir("palettes");
	dir.allowExt("png");
	dir.listDir();
	for(int i = 0; i < dir.numFiles(); i++){
		paletteNames.push_back(ofFilePath::removeExt( dir.getName(i) ));
		colorPalettes.push_back(ofImage());
		colorPalettes[i].loadImage( dir.getPath(i) );
	}
}

void testApp::selectPalette(){
	int tries = 10;
	int palette;
	do {
		palette = ofRandom(colorPalettes.size());
	} while(palette == currentPalette && tries-- > 0);
	currentPalette = palette;
}

void testApp::generateGeometry(){
	
	mesh.clear();

	int columns = 32;
	int vertsPerColumn = 3;
	float colStep = 640. / columns;
	float vertStep = colStep / (vertsPerColumn+1);
	bool skipping = false;
	
	cout << "creating mesh with colstep " << colStep << " and vert step " << vertStep << endl;
	
	int colorIndex = 0;
	for(int c = 0; c < columns; c++){
		//draw one column
		skipping = true;
		for(int y = 0; y < 480; y += vertStep){
			if(skipping){
				skipping = ofRandomuf() > .7;
				colorIndex = ofRandom(3);
			}
			else {
				
				for(int x = c * colStep; x < (c+1) * colStep; x += vertStep ) {
					
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
					
					//ofVec3f colvec = ofVec3f(col.r,col.g,col.b);
					ofVec3f colvec = ofVec3f(colorIndex+.5,.5, ofRandomuf());
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
				
				skipping = ofRandomuf() > .98;
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
	
	speedVarianceImage.allocate(640, 1, OF_IMAGE_COLOR);
	for (int i = 0; i < speedVarianceImage.getWidth(); i++) {
		speedVarianceImage.getPixels()[i*3] = ofMap(powf(ofRandomuf(),2.),0.0, 1.0, .1, 1.0);
		speedVarianceImage.getPixels()[i*3+1] = ofRandomuf() > .5 ? 0.0 : 1.0;
		speedVarianceImage.getPixels()[i*3+2] = 0.;
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
	sound.stop();

	string sceneToLoad;
	if(currentPortraitIndex > 1 && !easterEggPlayed && ofRandomuf() > .99){
		easterEggPlayed = true;
		cout << "LOADING EASTER EGG HEHEHEHE" << endl;
		sceneToLoad = ofRandomuf() > .5 ? "Portraits/CHANTAL" : "Portraits/JAMES";
	}
	else{
		sceneToLoad = paths[currentPortraitIndex];

	}
	
	if(shouldStopPlaying){
		playing = false;
		shouldStopPlaying = false;
		player.stop();
		music.stop();
		return;
	}
	
	if(!loadScene( sceneToLoad )){
		ofSystemAlertDialog("Couldn't load path " + sceneToLoad );
		ofLogError("testApp::switchPortrait") << "Failed to load portrait " << sceneToLoad;
		return;
	}

	player.getVideoPlayer()->play();
	player.getVideoPlayer()->setSpeed(.5);
	player.getVideoPlayer()->setVolume(0.);


	string soundPath = "Portraits/_SOUND/" + ofToLower(player.getScene().name) + ".aif";
	hasSound = sound.loadMovie(soundPath);
	
	if(hasSound){
		cout << "loaded sound " << soundPath << endl;
		sound.setLoopState(OF_LOOP_NONE);
		sound.play();
		
		string subtitlePath = "subtitles/" +player.getScene().name + "_p.srt";
		hasSubtitles = false;
		if(ofFile(subtitlePath).exists()){
			hasSubtitles = titles.load(subtitlePath);
		}
	}
	
	selectPalette();

	portraitChangedTime = ofGetElapsedTimef();
	nextPortraitTime = portraitChangedTime + timePerPortrait;

}

//--------------------------------------------------------------
void testApp::draw(){
	
	if(!playing){
		ofDrawBitmapString("WAITING TO PLAY: hit space bar to begin", ofGetWidth()*.4, ofGetHeight()/2);
	}
	
    if(player.isLoaded()){
        
		if(playFullScreen && fbo.getWidth() != 1920){
			fbo.allocate(1920, 1080, GL_RGB, 4);
		}
		fbo.begin();
		if(playFullScreen){
			ofClear(0);
		}
		else{
			ofClear(1., .1, .1, 1);
		}
		
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
//			if(playFullScreen){
//				screens[i]->getCameraRef().begin(ofRectangle(0,0,1920,1080));
//			}
//			else{
				screens[i]->getCameraRef().begin(ofRectangle(screens[i]->rect.x,
															 fbo.getHeight() - screens[i]->rect.y - screens[i]->rect.height,
															 screens[i]->rect.width,
															 screens[i]->rect.height));
//			}
			renderer.bindRenderer();
			
			renderer.getShader().setUniform1f("flowPosition", -flowSpeed * ofGetElapsedTimef());
			renderer.getShader().setUniform1f("brightness", screens[i]->brightness);
			renderer.getShader().setUniform1f("contrast", screens[i]->contrast);
			renderer.getShader().setUniform2f("headPosition",
											  headPositions[player.getScene().name].x,
											  headPositions[player.getScene().name].y);
			renderer.getShader().setUniformMatrix4f("colors", colormatrix);
			
			renderer.getShader().setUniform1f("headSphereRadius",headSphereRadius);
			renderer.getShader().setUniform1f("headEffectFalloff",headEffectFalloff);
			renderer.getShader().setUniform1f("varianceEffect",*screens[i]->varianceEffect);

			float attenuate = 1;
			if(!hasComposeMode){
				if(hasSound){
					attenuate = ofMap( sound.getDuration() - sound.getPosition()*sound.getDuration(), .5, 0, 1.0, 0.0, true)
							  * ofMap( ofGetElapsedTimef() - portraitChangedTime, 0, .5, .0, 1.0, true);
					
				}
				else{
					attenuate = ofMap( nextPortraitTime - ofGetElapsedTimef(), .5, 0, 1.0, 0.0, true)
							  * ofMap( ofGetElapsedTimef() - portraitChangedTime, 0, .5, .0, 1.0, true);
				}
			}

			renderer.getShader().setUniform1f("maxExtend",*screens[i]->maxExtend * attenuate);

			renderer.getShader().setUniform1f("extendThreshold",extendThreshold);
			renderer.getShader().setUniform1f("extendFalloff",extendFalloff);
			renderer.getShader().setUniform1f("pureColorThreshold",pureColorThreshold);
			renderer.getShader().setUniform1f("pureColorFlicker",pureColorFlickerPos);
			
			renderer.getShader().setUniformTexture("varianceTex",varianceImage, 3);
			renderer.getShader().setUniformTexture("speedVarianceTex",speedVarianceImage, 4);
			renderer.getShader().setUniformTexture("paletteTex", colorPalettes[currentPalette], 5);

			if(screens[i]->name.find("LED") != string::npos){
				renderer.getShader().setUniform4f("colorCorrect",0,0,0,0);
			}
			else{
				renderer.getShader().setUniform4f("colorCorrect",
												  facadeColorCorrectR,
												  facadeColorCorrectG,
												  facadeColorCorrectB,0);
			}
			
			mesh.draw();
			
			
			renderer.unbindRenderer();
			screens[i]->getCameraRef().end();
			
			if(hasSubtitles){
				ofPushStyle();
				ofEnableAlphaBlending();
				if(playFullScreen){
					ofRectangle fullScreenSubtitles(0,1080-90,1920,90);
					
					ofSetColor(0, 30);
					ofRect(fullScreenSubtitles);
					
					ofSetColor(0);
					titles.draw(fullScreenSubtitles.getCenter().x , fullScreenSubtitles.getMaxY()-63);
					
					ofSetColor(255);
					titles.draw(fullScreenSubtitles.getCenter().x+2, fullScreenSubtitles.getMaxY()-65);
				}
				else{
					ofSetColor(0, 30);
					ofRect(led1.rect.x,led1.rect.getMaxY()-50,led1.rect.width,50);
					ofRect(led2.rect.x,led2.rect.getMaxY()-50,led2.rect.width,50);
					
					ofSetColor(0);
					titles.draw(led1.rect.getCenter().x , led1.rect.getMaxY()-28);
					titles.draw(led2.rect.getCenter().x , led2.rect.getMaxY()-28);

					ofSetColor(255);
					titles.draw(led1.rect.getCenter().x+2, led1.rect.getMaxY()-30);
					titles.draw(led2.rect.getCenter().x+2, led2.rect.getMaxY()-30);
				}
				
				ofPopStyle();
			}
			
//			if (playFullScreen) {
//				break;
//			}
		}

		//MASK
		if(!playFullScreen){
			ofEnableAlphaBlending();
			for(int i = 0; i < screens.size(); i++){
				if(screens[i]->mask.isAllocated()){
					screens[i]->mask.draw(screens[i]->rect);
				}
			}
		}

		fbo.end();
		
		syphonServer.publishTexture(&fbo.getTextureReference());
		if(playFullScreen){
			ofBackground(0);
		}

		fbo.draw(0, 0, 1920,1080);
		
		
		if (playFullScreen) {
			return;
		}
    }

	ofDrawBitmapString("name " + player.getScene().name +
					   "\nfps " + ofToString(ofGetFrameRate()) +
					   "\ntime til next " + ofToString(nextPortraitTime - ofGetElapsedTimef(),2) +
					   "\npalette " + paletteNames[currentPalette],
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
	
	if(shouldStopPlaying){
		ofPushStyle();
		ofSetColor(255,200,150);
		ofDrawBitmapString("PLAYBACK WILLS TOP AFTER CURRENT PORTRAIT", led2.rect.x, led2.rect.getMaxY() + 25);
		ofPopStyle();
	}


}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	
    if(key == ' '){

		if(!playing){
			shouldStartPlaying = true;
		}
		else{
			shouldStopPlaying = true;
		}
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
	if(key == ']'){
		selectPalette();
	}
	if(key == 'S'){
		
		if(soundPlaying){
			sound.setVolume(0.0);
			music.setVolume(0.0);
		}
		else{
			sound.setVolume(1.0);
			music.setVolume(1.0);
		}
		soundPlaying = !soundPlaying;
	}
	
	if(key == 'F'){
		playFullScreen = !playFullScreen;
		ofToggleFullscreen();
		//mutate the screens
		
		led1.rect = ofRectangle(0, 0, 1920-720,540);
		led1.debugLocation.x = led1.rect.getMaxX();
		led1.debugLocation.y = led1.rect.getMaxY()-70;
		led1.maxExtend = &maxExtendLED;
		led1.varianceEffect = &varianceEffectLED;
		
		led2.rect = ofRectangle(0, 540, 1920-720,540);
		led2.debugLocation.x = led2.rect.getMaxX();
		led2.debugLocation.y = led2.rect.getY();
		led2.maxExtend = &maxExtendLED;
		led2.varianceEffect = &varianceEffectLED;
		
		centerFacade.rect = ofRectangle(1920-720, 0,720,1080);
		centerFacade.debugLocation.x = centerFacade.rect.getX();
		centerFacade.debugLocation.y = centerFacade.rect.getY()-60;
		centerFacade.maxExtend = &maxExtendFacade;
		centerFacade.varianceEffect = &varianceEffectFacade;

		titles.setup("subtitles/AxisStd-Regular.otf", 15);

		//delete left and right
		screens.erase( screens.begin() + 4);
		screens.erase( screens.begin() + 2);
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
