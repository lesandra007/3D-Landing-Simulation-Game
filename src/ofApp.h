#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include  "ofxAssimpModelLoader.h"
#include "Octree.h"
#include "Player.h"
#include "CameraSystem.h"
#include <glm/gtx/intersect.hpp>


class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void gotMessage(ofMessage msg);
		void drawAxis(ofVec3f);
		void initLightingAndMaterials();
		void savePicture();
		void toggleWireframeMode();
		void togglePointsDisplay();
		void toggleSelectTerrain();
		void setCameraTarget();
		bool mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point);
		bool raySelectWithOctree(ofVec3f &pointRet);
		glm::vec3 getMousePointOnPlane(glm::vec3 p , glm::vec3 n);
		float getAltitude();

		/* Cameras */
		CameraSystem cameraSystem = CameraSystem(false); //remember to change if using diff terrain
		void switchCameraMode(CameraSystem::CameraMode mode);
		//void drawCameraInfo();

		/* Lights */
		ofLight light;
		ofLight mainLight;          // Primary light source
		ofLight fillLight;          // Secondary fill light
		ofLight rimLight;           // Rim/back light for highlighting edges

		/* Models */
		ofxAssimpModelLoader mars;
		Box boundingBox, landerBounds;
		Box testBox;
		vector<Box> colBoxList;
		bool bLanderSelected = false;
		Octree octree;
		TreeNode selectedNode;
		glm::vec3 mouseDownPos, mouseLastPos;
		bool bInDrag = false;

		bool bLanderLoaded;
		bool bTerrainSelected;

		ofVec3f selectedPoint;
		ofVec3f intersectPoint;

		vector<Box> bboxList;

		const float selectionRange = 4.0;

		vector<ofColor> colors;

		bool bReverse = false;
		glm::vec3 landerLastPos;

		/* GUI */
		ofxIntSlider numLevels;
		ofxPanel gui;

		/* Keys */
		map<int, bool> keymap;
		bool bAltKeyDown;
		bool bCtrlKeyDown;
		bool bWireframe;
		bool bDisplayPoints;
		bool bPointSelected;
		bool bHide;
		bool pointSelected = false;
		bool bDisplayLeafNodes = false;
		bool bDisplayOctree = false;
		bool bDisplayBBoxes = false;
		
		/* Timing */
		ofxToggle timingToggle;
		bool bTimingInfo = true;

		// window dimensions
		int windowWidth;
		int windowHeight;

		// player
		Player player;


		/* Altitude */
		bool bShowAltitude = true;

		/* Fonts */
		ofTrueTypeFont font;
		char altitudeStr[30];
		char scoreStr[30];

		/* Fuel */
		ofColor fuelBarColor;
		ofColor fuelBarBgColor;
		int fuelBarWidth;
		int fuelBarHeight;
		int fuelBarPosX;
		int fuelBarPosY;

		/* Sound players */
		ofSoundPlayer thrustSound;
		ofSoundPlayer backgroundMusic;
		bool bThrustPlaying = false;
		bool isMoving = false;

		/* Background Image */
		ofImage backgroundImage;
		bool bBackgroundLoaded;

		/* Score */
		bool bShowScore = false;
};
