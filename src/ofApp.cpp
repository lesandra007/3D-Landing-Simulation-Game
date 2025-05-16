
//--------------------------------------------------------------
//
//  Kevin M. Smith
//
//  Octree Test - startup scene
// 
//
//  Student Name:   
//  Date: 


#include "ofApp.h"
#include "Util.h"


//--------------------------------------------------------------
// setup scene, lighting, state and load geometry
//
void ofApp::setup(){
	bWireframe = false;
	bDisplayPoints = false;
	bAltKeyDown = false;
	bCtrlKeyDown = false;
	bLanderLoaded = false;
	bTerrainSelected = true;
//	ofSetWindowShape(1024, 768);
	ofSetVerticalSync(true);
	cameraSystem.disableMouseInput();
	ofEnableSmoothing();
	ofEnableDepthTest();

	/* Font */
	if (!font.load("fonts/Electrolize-Regular.ttf", 20)) {
		cout << "can't load font" << endl;
		ofExit(0);
	}

	/* Fuel */
	// Initialize fuel bar position and dimensions
	fuelBarWidth = 20;
	fuelBarHeight = 200;
	fuelBarPosX = 50;
	fuelBarPosY = 100;
	fuelBarColor = ofColor::green;

	// setup rudimentary lighting 
	//
	initLightingAndMaterials();

	/* Terrain */
	//mars.loadModel("geo/mars-low-5x-v2.obj");
	//mars.loadModel("geo/moon-houdini.obj");
	mars.loadModel("geo/Mountain/Mountain.obj");
	//mars.loadModel("geo/Park/Park.obj");
	//mars.loadModel("geo/Alien/Alien.obj");

	mars.setScaleNormalization(false);

	/* Player */
	//player.lander.loadModel("geo/lander.obj");  // Load model
	player.lander.loadModel("geo/Missile/Missile.obj");
	player.setPosition(10, 50, 0);
	float scaleFactor = 0.25;
	player.scale = glm::vec3(scaleFactor, scaleFactor, scaleFactor);
	player.lander.setScaleNormalization(false);
	//player.lander.setRotation(0, -90, 0, 1, 0); // Rotate 90 degrees counterclockwise around Y-axis
	bLanderLoaded = true;


	/* Altitude */
	if (bShowAltitude) {
		sprintf(altitudeStr, "Altitude AGL: %f", getAltitude());
	}

	// create sliders/toggle for testing
	//
	gui.setup();
	gui.add(numLevels.setup("Number of Octree Levels", 1, 1, 10));
	gui.add(timingToggle.setup("Timing Info", true));
	
	bHide = false;

	// colors
	colors = {ofColor::red, ofColor::orange, ofColor::yellow, ofColor:: green, ofColor::blue, 
			  ofColor::purple, ofColor::brown, ofColor::gray, ofColor::magenta, ofColor::gold,
			  ofColor::beige, ofColor::mediumSeaGreen, ofColor::cyan, ofColor::indigo, ofColor::hotPink,
			  ofColor::chocolate, ofColor::white, ofColor::turquoise, ofColor::mintCream, ofColor::aqua};

	// Create Octree for testing.
	uint64_t startBuildTime = ofGetSystemTimeMillis();
	octree.create(mars.getMesh(0), 20);
	uint64_t endBuildTime = ofGetSystemTimeMillis();

	// calculate build time
	bTimingInfo = timingToggle;
	if (bTimingInfo) {
		cout << "Build Time: " << endBuildTime - startBuildTime << endl;
	}
	
	cout << "Number of Verts: " << mars.getMesh(0).getNumVertices() << endl;
	

	testBox = Box(Vector3(3, 3, 0), Vector3(5, 5, 2));

	/* Sound */
	// Load thrust sound for player movement
	thrustSound.load("geo/sound/thrust.wav");
	thrustSound.setLoop(true);
	thrustSound.setVolume(0.7f);

	// Load background music and set it to loop with reduced volume
	backgroundMusic.load("geo/sound/background.wav");
	backgroundMusic.setLoop(true);
	backgroundMusic.setVolume(0.3f); // Reduced volume for background music
	backgroundMusic.play(); // Start playing background music immediately

}
 
//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {
	/* Timing */
	// update boolean for timing info
	bTimingInfo = timingToggle;

	/* Player */
	bool isMoving = false; // Track if player is moving to control sound

	if (bLanderLoaded) {
		// rotate player counterclockwise
		if (keymap['a']) {
			//cout << "a key" << endl;
			player.rotateCounterclockwise();
			isMoving = true;
		}
		// rotate player clockwise
		if (keymap['d']) {
			//cout << "d key" << endl;
			player.rotateClockwise();
			isMoving = true;
		}
		// move player upward
		if (keymap['w']) {
			//cout << "w key" << endl;
			if (player.fuel > 0) {
				player.moveUp();
				isMoving = true;
			}
		}
		// move player downward
		if (keymap['s']) {
			//cout << "s key" << endl;
			if (player.fuel > 0) {
				player.moveDown();
				isMoving = true;
			}
		}
		// move forward along the heading vector
		if (keymap[OF_KEY_UP]) {
			// sound handled separately now
			//cout << "up arrow" << endl;
			if (player.fuel > 0) {
				player.moveForward();
				isMoving = true;
			}
		}
		// move backward along the heading vector
		if (keymap[OF_KEY_DOWN]) {
			// sound handled separately now
			//cout << "down arrow" << endl;
			if (player.fuel > 0) {
				player.moveBackward();
				isMoving = true;
			}
		}
		// move right of the heading vector 
		if (keymap[OF_KEY_RIGHT]) {
			if (player.fuel > 0) {
				player.moveRight();
				isMoving = true;
			}
		}
		// move left of the heading vector
		if (keymap[OF_KEY_LEFT]) {
			if (player.fuel > 0) {
				player.moveLeft();
				isMoving = true;
			}
		}

		// Sound handling for thrust
		if (isMoving) {
			if (!bThrustPlaying) {
				player.diskEmitter.sys->reset();
				player.diskEmitter.start();
				player.fuel -= player.fuelConsumptionRate; // Consume fuel
				if (player.fuel < 0) player.fuel = 0;
				thrustSound.play();
				bThrustPlaying = true;
			}
		}
		else {
			if (bThrustPlaying) {
				thrustSound.stop();
				bThrustPlaying = false;
			}
		}

		// Store last position for collision handling
		landerLastPos = player.getPosition();

		// Update camera positions based on player position
		if (cameraSystem.getCurrentMode() == CameraSystem::CameraMode::TRACKING_CAMERA) {
			cameraSystem.setCameraTarget(player.getPosition());
		}
		else if (cameraSystem.getCurrentMode() == CameraSystem::CameraMode::ONBOARD_CAMERA) {
			glm::vec3 playerPos = player.getPosition();
			glm::vec3 playerForward = player.heading();
			glm::vec3 playerUp = glm::vec3(0, 1, 0);
			cameraSystem.updateOnboardCamera(playerPos, playerForward, playerUp);
		}
	}

	/* Collision */
	// if model is in collided state
	if (bReverse) {
		// get lander's current position and velocity
		glm::vec3 currPos = player.lander.getPosition();
		glm::vec3 landerVelocity = currPos - landerLastPos;
		
		// interval for this step
		float dt = 1.0 / ofGetFrameRate();

		// get normal for the vertex in the mesh where lander is in contact with
		glm::vec3 collisionNormalSum = glm::vec3(0, 0, 0);
		float numPoints = 0;
		// for each contact point, get the normal
		for (Box box : colBoxList) {
			// points in colided mesh's bounding boxes
			vector<int> pointsRtn;
			octree.getMeshPointsInBox(octree.mesh, octree.root.points, box, pointsRtn);
			for (int i = 0; i < pointsRtn.size(); i++) {
				collisionNormalSum += octree.mesh.getNormal(pointsRtn[i]);
			}
			numPoints += pointsRtn.size();
		}
		// calculate the averge normal from all the contact points
		glm::vec3 avgCollisionNormal = glm::normalize(collisionNormalSum / numPoints);

		// calculate new velocity
		glm::vec3 reverseVelocity = glm::reflect(landerVelocity, avgCollisionNormal);
		// ensure the direction of the new velocity  is away from the terrain
		if (glm::dot(reverseVelocity, avgCollisionNormal) < 0) {
			reverseVelocity *= -1.0f;
		}
		glm::vec3 newVelocity = reverseVelocity * 2.0f;

		// update lander's position
		glm::vec3 newPos = currPos + reverseVelocity * dt;
		player.setPosition(newPos.x, newPos.y, newPos.z);

		// lander bounds
		ofVec3f min = player.lander.getSceneMin() + player.getPosition();
		ofVec3f max = player.lander.getSceneMax() + player.getPosition();
		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

		// update collision box list
		colBoxList.clear();
		octree.intersect(bounds, octree.root, colBoxList);

		// if no longer in collision state, stop reversing
		if (colBoxList.size() < 10) {
			bReverse = false;
		}
	}

	player.update();

	/* Altitude */
	if (bShowAltitude) {
		float agl = getAltitude();
		if (agl == -1) {
			sprintf(altitudeStr, "Altitude AGL: -------");
		}
		else {
			sprintf(altitudeStr, "Altitude AGL: %f", getAltitude());
		}
	}
}
//--------------------------------------------------------------
void ofApp::draw() {
	//ofBackground(ofColor::black);

	cameraSystem.begin();
	ofPushMatrix();
	if (bWireframe) {                    // wireframe mode  (include axis)
		ofDisableLighting();
		ofSetColor(ofColor::slateGray);
		mars.drawWireframe();
		if (bLanderLoaded) {
			player.drawWireframe();
			if (!bTerrainSelected) drawAxis(player.getPosition());
		}
		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
	}
	else {
		ofEnableLighting();              // shaded mode
		mars.drawFaces();
		ofMesh mesh;
		if (bLanderLoaded) {
			player.draw();
			if (!bTerrainSelected) drawAxis(player.getPosition());
			if (bDisplayBBoxes) {
				ofNoFill();
				ofSetColor(ofColor::white);
				for (int i = 0; i < player.lander.getNumMeshes(); i++) {
					ofPushMatrix();
					ofMultMatrix(player.getTransform()); //lander.getModelMatrix()
					ofRotate(-90, 1, 0, 0); // ensure rotation is right
					Octree::drawBox(bboxList[i]);
					ofPopMatrix();
				}
			}

			if (bLanderSelected) {

				ofVec3f min = player.lander.getSceneMin() + player.getPosition();
				ofVec3f max = player.lander.getSceneMax() + player.getPosition();

				Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
				ofSetColor(ofColor::white);
				Octree::drawBox(bounds);

				// draw colliding boxes
				//
				ofSetColor(ofColor::lightBlue);
				for (int i = 0; i < colBoxList.size(); i++) {
					Octree::drawBox(colBoxList[i]);
				}
			}
		}
	}
	if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));

	// recursively draw octree
	//
	ofDisableLighting();
	int level = 0;
	//	ofNoFill();

	if (bDisplayLeafNodes) {
		octree.drawLeafNodes(octree.root);
		cout << "num leaf: " << octree.numLeaf << endl;
    }
	else if (bDisplayOctree) {
		ofNoFill();
		ofSetColor(ofColor::white);
		octree.draw(numLevels, 0, colors);
	}

	ofPopMatrix();
	cameraSystem.end();

	glDepthMask(false);
	if (!bHide) gui.draw();
	if (bShowAltitude) {
		ofSetColor(ofColor::white);
		font.drawString(altitudeStr, 500, 50);
	}

	if (bLanderLoaded) {
		// Draw background (gray)
		ofSetColor(ofColor::gray);
		ofFill();
		ofDrawRectangle(fuelBarPosX, fuelBarPosY, fuelBarWidth, fuelBarHeight);

		// Draw fuel level
		float fuelLevel = player.getFuelPercentage();
		
		// Change color based on fuel level
		if (fuelLevel > 0.6f) {
			fuelBarColor = ofColor::green;
		}
		else if (fuelLevel > 0.3f) {
			fuelBarColor = ofColor::yellow;
		}
		else {
			fuelBarColor = ofColor::red;
		}

		// Draw fuel from the bottom of the bar
		float filledHeight = fuelBarHeight * fuelLevel;
		float filledY = fuelBarPosY + fuelBarHeight - filledHeight;
		ofSetColor(fuelBarColor);

		ofDrawRectangle(fuelBarPosX, filledY, fuelBarWidth, filledHeight);

		// Draw border
		ofNoFill();
		ofSetColor(ofColor::white);
		ofDrawRectangle(fuelBarPosX, fuelBarPosY, fuelBarWidth, fuelBarHeight);
		ofFill();

		font.drawString("Fuel", fuelBarPosX - 20, fuelBarPosY - 7);
	}

	glDepthMask(true);
}


// 
// Draw an XYZ axis in RGB at world (0,0,0) for reference.
//
void ofApp::drawAxis(ofVec3f location) {

	ofPushMatrix();
	ofTranslate(location);

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));
	

	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

	ofPopMatrix();
}


void ofApp::keyPressed(int key) {

	switch (key) {
	case '1':
		// Switch to EasyCam (free camera)
		switchCameraMode(CameraSystem::CameraMode::EASYCAM);
		break;
	case '2':
		// Switch to tracking camera
		switchCameraMode(CameraSystem::CameraMode::TRACKING_CAMERA);
		break;
	case '3':
		// Switch to onboard camera
		switchCameraMode(CameraSystem::CameraMode::ONBOARD_CAMERA);
		break;
	case 'B':
	case 'b':
		bDisplayBBoxes = !bDisplayBBoxes;
		break;
	case 'C':
	case 'c':
		if (cameraSystem.getMouseInputEnabled()) cameraSystem.disableMouseInput();
		else cameraSystem.enableMouseInput();
		break;
	case 'e':
		bShowAltitude = !bShowAltitude;
		break;
	case 'F':
	case 'f':
		ofToggleFullscreen();
		break;
	case 'H':
	case 'h':
		break;
	case 'L':
	case 'l':
		bDisplayLeafNodes = !bDisplayLeafNodes;
		break;
	case 'O':
	case 'o':
		bDisplayOctree = !bDisplayOctree;
		break;
	case 'q':
		player.breakPlayer();
		break;
	case 'r':
		cameraSystem.reset();
		break;
	case 's':
		//savePicture();
		break;
	case 't':
		setCameraTarget();
		break;
	case 'u':
		break;
	case 'v':
		togglePointsDisplay();
		break;
	case 'V':
		break;
	case 'w':
		//toggleWireframeMode();
		break;
	case OF_KEY_ALT:
		cameraSystem.enableMouseInput();
		bAltKeyDown = true;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = true;
		break;
	case OF_KEY_SHIFT:
		break;
	case OF_KEY_DEL:
		break;
	case ' ':
		// if lander is loaded and in collision state, reverse the lander
		if (bLanderLoaded && colBoxList.size() >= 10) {
			bReverse = true;
		}
	default:
		break;
	}
	
	// for player controls
	keymap[key] = true;
}

void ofApp::toggleWireframeMode() {
	bWireframe = !bWireframe;
}

void ofApp::toggleSelectTerrain() {
	bTerrainSelected = !bTerrainSelected;
}

void ofApp::togglePointsDisplay() {
	bDisplayPoints = !bDisplayPoints;
}

void ofApp::keyReleased(int key) {

	switch (key) {
	
	case OF_KEY_ALT:
		cameraSystem.disableMouseInput();
		bAltKeyDown = false;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = false;
		break;
	case OF_KEY_SHIFT:
		break;
	default:
		break;

	}

	// for player controls
	keymap[key] = false;
}



//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

	
}

glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 planePt, glm::vec3 planeNorm) {
	// Setup our rays
	//
	if (cameraSystem.currentMode == CameraSystem::CameraMode::EASYCAM) {
		glm::vec3 origin = cameraSystem.getEasyCam().getPosition();
		glm::vec3 camAxis = cameraSystem.getEasyCam().getZAxis();
		glm::vec3 mouseWorld = cameraSystem.getEasyCam().screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
		float distance;

		bool hit = glm::intersectRayPlane(origin, mouseDir, planePt, planeNorm, distance);

		if (hit) {
			// find the point of intersection on the plane using the distance 
			// We use the parameteric line or vector representation of a line to compute
			//
			// p' = p + s * dir;
			//
			glm::vec3 intersectPoint = origin + distance * mouseDir;

			return intersectPoint;
		}
	}
	else {
		cout << "Must be in easy cam to use this feature";
		return glm::vec3(0, 0, 0);
	}
}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (cameraSystem.getMouseInputEnabled()) return;

	// if moving camera, don't allow mouse interaction
//
	if (cameraSystem.getMouseInputEnabled()) return;

	// if rover is loaded, test for selection
	//
	if (bLanderLoaded) {
		if (cameraSystem.currentMode == CameraSystem::CameraMode::EASYCAM) {
			glm::vec3 origin = cameraSystem.getEasyCam().getPosition();
			glm::vec3 mouseWorld = cameraSystem.getEasyCam().screenToWorld(glm::vec3(mouseX, mouseY, 0));
			glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);

			ofVec3f min = player.lander.getSceneMin() + player.getPosition();
			ofVec3f max = player.lander.getSceneMax() + player.getPosition();

			Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

			bool hit = bounds.intersect(Ray(Vector3(origin.x, origin.y, origin.z), Vector3(mouseDir.x, mouseDir.y, mouseDir.z)), 0, 10000);
			if (hit) {
				bLanderSelected = true;
				mouseDownPos = getMousePointOnPlane(player.getPosition(), cameraSystem.getEasyCam().getZAxis());
				mouseLastPos = mouseDownPos;
				bInDrag = true;
			}
			else {
				bLanderSelected = false;
			}
		}
		else {
			cout << "Must be in easy cam to use this feature!";
		}
	}
}




//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (cameraSystem.getMouseInputEnabled()) return;
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	bInDrag = false;
}



// Set the camera to use the selected point as it's new target
//-------------------------------------------------------------
void ofApp::setCameraTarget() {
	if (bLanderLoaded) {
		cameraSystem.setCameraTarget(player.getPosition());
	}
}


//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
	// update window dimension variables
	windowWidth = w;
	windowHeight = h;
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}



//--------------------------------------------------------------
// setup basic ambient lighting in GL  (for now, enable just 1 light)
//
void ofApp::initLightingAndMaterials() {
	// Common ambient and diffuse properties
	static float ambient[] = { .5f, .5f, .5f, 1.0f };
	static float diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	static float lmodel_ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	static float lmodel_twoside[] = { GL_TRUE };

	// Main light (LIGHT0) - Key light
	static float position0[] = { 100.0f, 120.0f, 80.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position0);

	// Fill light (LIGHT1) - Softer, fills shadows
	static float ambient1[] = { 0.3f, 0.3f, 0.4f, 1.0f };
	static float diffuse1[] = { 0.6f, 0.6f, 0.8f, 1.0f };
	static float position1[] = { -150.0f, 70.0f, 10.0f, 0.0f };
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse1);
	glLightfv(GL_LIGHT1, GL_POSITION, position1);

	// Rim light (LIGHT2) - Highlights edges, creates separation
	static float ambient2[] = { 0.2f, 0.15f, 0.1f, 1.0f };
	static float diffuse2[] = { 0.8f, 0.7f, 0.55f, 1.0f };
	static float position2[] = { -30.0f, 50.0f, -180.0f, 0.0f };
	glLightfv(GL_LIGHT2, GL_AMBIENT, ambient2);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, diffuse2);
	glLightfv(GL_LIGHT2, GL_POSITION, position2);

	// Global lighting model settings
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	// Enable lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);

	glShadeModel(GL_SMOOTH);
}

void ofApp::savePicture() {
	ofImage picture;
	picture.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
	picture.save("screenshot.png");
	cout << "picture saved" << endl;
}


/* Returns player's altitude above ground level */
float ofApp::getAltitude() {
	// return negative if player has exploded
	if (!player.isVisible()) { return -1; }

	glm::vec3 origin = player.getPosition();
	glm::vec3 dir = glm::vec3(0, -1, 0); // downward direction

	TreeNode nodeBelow;

	bool aboveTerrain = octree.intersect(Ray(Vector3(origin.x, origin.y, origin.z), Vector3(dir.x, dir.y, dir.z)), octree.root, nodeBelow);

	// if above terrain, get distance between player and terrain

	if (aboveTerrain) {
		ofVec3f pointRet = octree.mesh.getVertex(nodeBelow.points[0]);
		return origin.y - pointRet.y;
	}
	else {
		return -1;
	}
}

// Switch between camera modes
void ofApp::switchCameraMode(CameraSystem::CameraMode mode) {
	// Store current camera mode
	CameraSystem::CameraMode prevMode = cameraSystem.getCurrentMode();

	// Update camera mode
	cameraSystem.switchCameraMode(mode);

	// If switching to onboard camera, make sure it's positioned correctly
	if (mode == CameraSystem::CameraMode::ONBOARD_CAMERA && bLanderLoaded) {
		// Update onboard camera position and orientation based on player
		glm::vec3 playerPos = player.getPosition();
		glm::vec3 playerForward = player.heading();
		glm::vec3 playerUp = glm::vec3(0, 1, 0); // Assuming Y is up
		cameraSystem.updateOnboardCamera(playerPos, playerForward, playerUp);
	}

	// If switching to tracking camera, make sure it's looking at the player
	if (mode == CameraSystem::CameraMode::TRACKING_CAMERA && bLanderLoaded) {
		cameraSystem.setCameraTarget(player.getPosition());
	}

	// If switching from EasyCam to another mode, disable mouse input
	if (prevMode == CameraSystem::CameraMode::EASYCAM && cameraSystem.getMouseInputEnabled()) {
		cameraSystem.disableMouseInput();
	}
}