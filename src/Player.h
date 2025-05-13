#pragma once

#include "Shape.h"
#include "ofxAssimpModelLoader.h"
#include "ParticleEmitter.h"

/* A Shape that can be moved with physics */
class Player : public DynamicShape {
public:
	// methods
	Player() {
		// dimensions
		width = 60.0f;
		height = 70.0f;
		radius = height;
		// physics
		speed = 200.0f;  //100 for lander
		torque = 100.f;   //50 for lander
	}

	/* Deallocate the emitter when player is deleted */
	~Player() {
		delete emitter;
	}

	/* Draws the lander */
	void draw() {
		ofPushMatrix();
		ofMultMatrix(getTransform());
		lander.drawFaces();
		ofPopMatrix();

	}

	/* Draws the lander's wireframe */
	void drawWireframe() {
		ofPushMatrix();
		ofMultMatrix(getTransform());
		lander.drawWireframe();
		ofPopMatrix();

	}

	void setPosition(float x, float y, float z) {
		pos = glm::vec3(x, y, z);
	}

	glm::vec3 getPosition() {
		return pos;
	}

	/* Move the player with physics */
	void update() {
		if (ofGetFrameRate() != 0) {
			integrate();
		}
	}

	ofxAssimpModelLoader lander;
	ParticleEmitter* emitter;
};
