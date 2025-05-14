
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
	cam.setDistance(60);
	cam.setNearClip(.1);
	cam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
	ofSetVerticalSync(true);
	cam.disableMouseInput();
	ofEnableSmoothing();
	ofEnableDepthTest();

	/* Font */
	if (!font.load("fonts/Electrolize-Regular.ttf", 20)) {
		cout << "can't load font" << endl;
		ofExit(0);
	}

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
	mars.loadModel("geo/mars-low-5x-v2.obj");
	//mars.loadModel("geo/moon-houdini.obj");
	// 
	//mars.loadModel("geo/Mountain/Mountain.fbx");
	//mars.loadModel("geo/Park/Park.obj");

	mars.setScaleNormalization(false);

	/* Player */
	player.lander.loadModel("geo/lander.obj");  // Load model
	//player.lander.loadModel("geo/Missile/Missile.obj");
	player.lander.setScaleNormalization(false);
	//player.lander.setRotation(0, -90, 0, 1, 0); // Rotate 90 degrees counterclockwise around Y-axis
	bLanderLoaded = true;
	player.setPosition(0, 10, 0); // Set initial position

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

}
 
//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {
	/* Timing */
	// update boolean for timing info
	bTimingInfo = timingToggle;

	/* Player */
	if (bLanderLoaded) {
		// rotate player counterclockwise
		if (keymap['a']) {
			//cout << "a key" << endl;
			player.rotateCounterclockwise();
		}
		// rotate player clockwise
		if (keymap['d']) {
			//cout << "d key" << endl;
			player.rotateClockwise();
		}
		// move player upward
		if (keymap['w']) {
			//cout << "w key" << endl;
			if (player.fuel > 0) {
				player.moveUp();
				player.fuel -= player.fuelConsumptionRate; // Consume fuel
				if (player.fuel < 0) player.fuel = 0;
			}
		}
		// rotate player clockwise
		if (keymap['s']) {
			//cout << "s key" << endl;
			if (player.fuel > 0) {
				player.moveDown();
				player.fuel -= player.fuelConsumptionRate; // Consume fuel
				if (player.fuel < 0) player.fuel = 0;
			}
		}
		// move forward along the heading vector
		if (keymap[OF_KEY_UP]) {
			// sound
			//engine.play();
			//cout << "up arrow" << endl;
			if (player.fuel > 0) {
				player.moveForward();
				player.fuel -= player.fuelConsumptionRate; // Consume fuel
				if (player.fuel < 0) player.fuel = 0;
			}
		}
		// move backward along the heading vector
		if (keymap[OF_KEY_DOWN]) {
			// sound
			//engine.play();
			//cout << "down arrow" << endl;
			if (player.fuel > 0) {
				player.moveBackward();
				player.fuel -= player.fuelConsumptionRate; // Consume fuel
				if (player.fuel < 0) player.fuel = 0;
			}
		}
		// move right of the heading vector 
		if (keymap[OF_KEY_RIGHT]) {
			if (player.fuel > 0) {
				player.moveRight();
				player.fuel -= player.fuelConsumptionRate; // Consume fuel
				if (player.fuel < 0) player.fuel = 0;
			}
		}
		// move left of the heading vector
		if (keymap[OF_KEY_LEFT]) {
			if (player.fuel > 0) {
				player.moveLeft();
				player.fuel -= player.fuelConsumptionRate; // Consume fuel
				if (player.fuel < 0) player.fuel = 0;
			}
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

	cam.begin();
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

	// if point selected, draw a sphere
	//
	if (pointSelected) {
		ofVec3f p = octree.mesh.getVertex(selectedNode.points[0]);
		ofVec3f d = p - cam.getPosition();
		ofSetColor(ofColor::lightGreen);
		ofDrawSphere(p, .02 * d.length());
	}
	ofPopMatrix();
	cam.end();

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
	case 'B':
	case 'b':
		bDisplayBBoxes = !bDisplayBBoxes;
		break;
	case 'C':
	case 'c':
		if (cam.getMouseInputEnabled()) cam.disableMouseInput();
		else cam.enableMouseInput();
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
		cam.reset();
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
		cam.enableMouseInput();
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
		cam.disableMouseInput();
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


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (cam.getMouseInputEnabled()) return;

	// if moving camera, don't allow mouse interaction
//
	if (cam.getMouseInputEnabled()) return;

	// if rover is loaded, test for selection
	//
	if (bLanderLoaded) {
		glm::vec3 origin = cam.getPosition();
		glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);

		ofVec3f min = player.lander.getSceneMin() + player.getPosition();
		ofVec3f max = player.lander.getSceneMax() + player.getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

		bool hit = bounds.intersect(Ray(Vector3(origin.x, origin.y, origin.z), Vector3(mouseDir.x, mouseDir.y, mouseDir.z)), 0, 10000);
		if (hit) {
			bLanderSelected = true;
			mouseDownPos = getMousePointOnPlane(player.getPosition(), cam.getZAxis());
			mouseLastPos = mouseDownPos;
			bInDrag = true;
		}
		else {
			bLanderSelected = false;
		}
	}
	else {
		ofVec3f p;
		raySelectWithOctree(p);
	}
}

bool ofApp::raySelectWithOctree(ofVec3f &pointRet) {
	ofVec3f mouse(mouseX, mouseY);
	ofVec3f rayPoint = cam.screenToWorld(mouse);
	ofVec3f rayDir = rayPoint - cam.getPosition();
	rayDir.normalize();
	Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
		Vector3(rayDir.x, rayDir.y, rayDir.z));

	pointSelected = octree.intersect(ray, octree.root, selectedNode);

	if (pointSelected) {
		pointRet = octree.mesh.getVertex(selectedNode.points[0]);
	}
	return pointSelected;
}




//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (cam.getMouseInputEnabled()) return;

	if (bInDrag) {

		glm::vec3 landerPos = player.getPosition();
		glm::vec3 mousePos = getMousePointOnPlane(landerPos, cam.getZAxis());
		glm::vec3 delta = mousePos - mouseLastPos;
	
		// update lander's last position
		landerLastPos = landerPos;
		landerPos += delta;
		player.setPosition(landerPos.x, landerPos.y, landerPos.z);
		mouseLastPos = mousePos;
		

		ofVec3f min = player.lander.getSceneMin() + player.getPosition();
		ofVec3f max = player.lander.getSceneMax() + player.getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

		colBoxList.clear();
		octree.intersect(bounds, octree.root, colBoxList);
	

		/*if (bounds.overlap(testBox)) {
			cout << "overlap" << endl;
		}
		else {
			cout << "OK" << endl;
		}*/


	}
	else {
		ofVec3f p;
		raySelectWithOctree(p);
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	bInDrag = false;
}



// Set the camera to use the selected point as it's new target
//  
void ofApp::setCameraTarget() {

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

	static float ambient[] =
	{ .5f, .5f, .5, 1.0f };
	static float diffuse[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float position[] =
	{5.0, 5.0, 5.0, 0.0 };

	static float lmodel_ambient[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float lmodel_twoside[] =
	{ GL_TRUE };


	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position);


	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
//	glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);
} 

void ofApp::savePicture() {
	ofImage picture;
	picture.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
	picture.save("screenshot.png");
	cout << "picture saved" << endl;
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent2(ofDragInfo dragInfo) {

	ofVec3f point;
	mouseIntersectPlane(ofVec3f(0, 0, 0), cam.getZAxis(), point);
	if (player.lander.loadModel(dragInfo.files[0])) {
		player.lander.setScaleNormalization(false);
//		lander.setScale(.1, .1, .1);
	//	lander.setPosition(point.x, point.y, point.z);
		player.setPosition(1, 1, 0);

		bLanderLoaded = true;
		for (int i = 0; i < player.lander.getMeshCount(); i++) {
			bboxList.push_back(Octree::meshBounds(player.lander.getMesh(i)));
		}

		cout << "Mesh Count: " << player.lander.getMeshCount() << endl;
	}
	else cout << "Error: Can't load model" << dragInfo.files[0] << endl;
}

bool ofApp::mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point) {
	ofVec2f mouse(mouseX, mouseY);
	ofVec3f rayPoint = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	ofVec3f rayDir = rayPoint - cam.getPosition();
	rayDir.normalize();
	return (rayIntersectPlane(rayPoint, rayDir, planePoint, planeNorm, point));
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent(ofDragInfo dragInfo) {
	if (player.lander.loadModel(dragInfo.files[0])) {
		bLanderLoaded = true;
		player.lander.setScaleNormalization(false);
		player.setPosition(0, 0, 0);
		// update lander last position
		landerLastPos = player.getPosition();
		cout << "number of meshes: " << player.lander.getNumMeshes() << endl;
		bboxList.clear();
		for (int i = 0; i < player.lander.getMeshCount(); i++) {
			bboxList.push_back(Octree::meshBounds(player.lander.getMesh(i)));
		}

		//		lander.setRotation(1, 180, 1, 0, 0);

				// We want to drag and drop a 3D object in space so that the model appears 
				// under the mouse pointer where you drop it !
				//
				// Our strategy: intersect a plane parallel to the camera plane where the mouse drops the model
				// once we find the point of intersection, we can position the lander/lander
				// at that location.
				//

				// Setup our rays
				//
		glm::vec3 origin = cam.getPosition();
		glm::vec3 camAxis = cam.getZAxis();
		glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
		float distance;

		bool hit = glm::intersectRayPlane(origin, mouseDir, glm::vec3(0, 0, 0), camAxis, distance);
		if (hit) {
			// find the point of intersection on the plane using the distance 
			// We use the parameteric line or vector representation of a line to compute
			//
			// p' = p + s * dir;
			//
			glm::vec3 intersectPoint = origin + distance * mouseDir;

			// Now position the lander's origin at that intersection point
			//
			glm::vec3 min = player.lander.getSceneMin();
			glm::vec3 max = player.lander.getSceneMax();
			float offset = (max.y - min.y) / 2.0;
			player.setPosition(intersectPoint.x, intersectPoint.y - offset, intersectPoint.z);

			// set up bounding box for lander while we are at it
			//
			landerBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		}
	}


}

//  intersect the mouse ray with the plane normal to the camera 
//  return intersection point.   (package code above into function)
//
glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 planePt, glm::vec3 planeNorm) {
	// Setup our rays
	//
	glm::vec3 origin = cam.getPosition();
	glm::vec3 camAxis = cam.getZAxis();
	glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
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
	else return glm::vec3(0, 0, 0);
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