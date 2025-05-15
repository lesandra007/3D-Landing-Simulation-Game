#pragma once

#include "Shape.h"
#include "ofxAssimpModelLoader.h"
#include "ParticleEmitter.h"
#include "vector3.h"
#include "box.h"

/* A Shape that can be moved with physics */
class Player : public DynamicShape {
public:
	// Fuel system
	float fuel;
	float maxFuel;
	float fuelConsumptionRate;

	// methods
	Player() {
		maxFuel = 100.0f;
		fuel = maxFuel;
		fuelConsumptionRate = 1.0f;
		// dimensions
		width = 60.0f;
		height = 70.0f;
		radius = height;
		// physics
		speed = 200.0f;
		torque = 100.f;

		// First initialize emitter.sys to make sure it exists before adding forces
		emitter.sys = new ParticleSystem();

		/* Emitter and Particles */
		// then set up the emitter forces
		turbForce = new TurbulenceForce(ofVec3f(-20, -20, -20), ofVec3f(20, 20, 20));
		gravityForce = new GravityForce(ofVec3f(0, -10, 0));
		radialForce = new ImpulseRadialForce(5);

		// Now add forces to the properly initialized system
		emitter.sys->addForce(turbForce);
		emitter.sys->addForce(gravityForce);
		emitter.sys->addForce(radialForce);

		// set up emitter
		emitter.speed = speed;
		emitter.torque = torque;
		emitter.setPosition(ofVec3f(0, 100, 0)); // Start with a safe position
		emitter.setVelocity(velocity);
		emitter.acceleration = acceleration;
		emitter.forces = forces;
		emitter.rot = rot;
		emitter.angVelocity = angVelocity;
		emitter.angAcceleration = angAcceleration;
		emitter.rotForces = rotForces;
		emitter.setOneShot(true);
		emitter.setEmitterType(RadialEmitter);
		emitter.setGroupSize(20);
		emitter.visible = false;

		// visibility: true if hasnt collided, otherwise false
		visible = true;
	}

	float getFuelPercentage() {
		return fuel / maxFuel;
	}

	/* Draws the lander */
	void draw() {
		ofPushMatrix();
		ofMultMatrix(getTransform());
		if (visible) {
			lander.drawFaces();
		}
		emitter.draw();
		ofPopMatrix();

	}

	/* Draws the lander's wireframe */
	void drawWireframe() {
		ofPushMatrix();
		ofMultMatrix(getTransform());
		lander.drawWireframe();
		ofPopMatrix();

	}

	/* Return player visibility: true if hasnt collided into anything yet, otherwise false */
	bool isVisible() {
		return visible;
	}

	/* Set player visibility */
	void setVisible(bool state) {
		visible = state;
	}

	/* Simulate a collision */
	void breakPlayer() {
		cout << "breakPlayer" << endl;
		visible = false;
		emitter.sys->reset();
		emitter.start();
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
			ofVec3f center = getCenter();
			emitter.forces = forces;

			emitter.rotForces = rotForces;
			emitter.integrate();
			emitter.update();
		}
	}

	ofVec3f getCenter() {
		if (!lander.hasMeshes()) {
			return ofVec3f(pos.x, pos.y, pos.z);  // Return position if model not loaded
		}
		ofVec3f min = lander.getSceneMin();
		ofVec3f max = lander.getSceneMax();
		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		Vector3 center = bounds.center();
		ofVec3f centerNew(center.x(), center.y() + pos.y, center.z());
		return centerNew;
	}

	ofxAssimpModelLoader lander;
	bool visible;

	ParticleEmitter emitter;
	TurbulenceForce* turbForce;
	GravityForce* gravityForce;
	ImpulseRadialForce* radialForce;
};
