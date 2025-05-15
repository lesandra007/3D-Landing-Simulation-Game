#pragma once

#include "Shape.h"
#include "ofxAssimpModelLoader.h"
#include "ParticleEmitter.h"
#include "vector3.h"
#include "box.h"

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

		/* Emitter and Particles */

		// set up the emitter forces
		turbForce = new TurbulenceForce(ofVec3f(-20, -20, -20), ofVec3f(20, 20, 20));
		gravityForce = new GravityForce(ofVec3f(0, -10, 0));
		radialForce = new ImpulseRadialForce(20);


		/* Collision Radial Emitter */
		emitter.sys->addForce(turbForce);
		emitter.sys->addForce(gravityForce);
		emitter.sys->addForce(radialForce);

		// set up emitter
		emitter.speed = speed * 5;
		emitter.torque = torque;
		emitter.mass = 1;

		emitter.setPosition(getCenter());
		emitter.setVelocity(velocity);
		emitter.acceleration = acceleration;
		emitter.forces = forces;
		

		emitter.rot = rot;
		emitter.angVelocity = angVelocity;
		emitter.angAcceleration = angAcceleration;
		emitter.rotForces = rotForces;

		emitter.setOneShot(true);
		emitter.setEmitterType(RadialEmitter);
		emitter.setGroupSize(5000);
		emitter.visible = true;

		/* Thrust Disk Emitter */
		diskEmitter.sys->addForce(gravityForce);

		// set up emitter
		diskEmitter.speed = 50;
		diskEmitter.torque = torque;
		diskEmitter.mass = 10;

		diskEmitter.radius = 2;

		diskEmitter.setPosition(pos);
		diskEmitter.setVelocity(velocity);
		diskEmitter.acceleration = acceleration;
		diskEmitter.forces = forces;


		diskEmitter.rot = rot;
		diskEmitter.angVelocity = angVelocity;
		diskEmitter.angAcceleration = angAcceleration;
		diskEmitter.rotForces = rotForces;

		diskEmitter.setOneShot(true);
		diskEmitter.setEmitterType(DiskEmitter);
		diskEmitter.setGroupSize(1000);
		diskEmitter.setParticleRadius(1);
		diskEmitter.visible = true;
		
		// visibility: true if hasnt collided, otherwise false
		visible = true;
	}

	/* Draws the lander */
	void draw() {
		ofPushMatrix();
		ofMultMatrix(getTransform());
		if (visible) {
			lander.drawFaces();
		}
		emitter.draw();
		diskEmitter.draw();
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
		diskEmitter.visible = state;
	}

	/* Simulate a collision */
	void breakPlayer() {
		cout << "breakPlayer" << endl;
		setVisible(false);
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

			/* Collision Radial Emitter */
			// emitter movement
			emitter.forces = forces;
			emitter.rotForces = rotForces;
			emitter.integrate();

			// spawn particles accordingly
			emitter.update();

			/* Thrust Disk Emitter */

			// emitter movement
			diskEmitter.forces = forces;
			diskEmitter.rotForces = rotForces;
			diskEmitter.integrate();

			// spawn particles accordingly
			if (visible) {
				diskEmitter.update();
			}
			
		}
	}

	ofVec3f getCenter() {
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
	ParticleEmitter diskEmitter;
	TurbulenceForce* turbForce;
	GravityForce* gravityForce;
	ImpulseRadialForce* radialForce;
};
