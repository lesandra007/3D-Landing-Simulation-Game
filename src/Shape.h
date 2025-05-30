#pragma once

#include "ofMain.h"


//--------------------------------------------------------------

// Basic Shape class supporting matrix transformations and drawing.
// 
//
class Shape {
public:
	Shape() {


	}

	/* Draws the default shape */
	virtual void draw() {

		// draw a sphere by default if not overridden
		//
		ofPushMatrix();
		ofMultMatrix(getTransform());
		ofDrawSphere(radius);
		ofPopMatrix();
	}

	/* Determine if the specified point is in the bounds of the shape */
	virtual bool inside(glm::vec3 p0) {
		float distance = glm::distance(pos, p0);
		return distance <= radius;
	}

	/* Returns the transformation matrix based on position, rotation, and scale */
	glm::mat4 getTransform() {
		// Translate to position
		glm::mat4 T = glm::translate(glm::mat4(1.0), glm::vec3(pos));
		// Rotate around y-axis
		glm::mat4 R = glm::rotate(glm::mat4(1.0), glm::radians(rot), glm::vec3(0, 1, 0));
		// Scale by scale variable
		glm::mat4 S = glm::scale(glm::mat4(1.0), scale);
		return T*R*S;
	}
	glm::vec3 pos = glm::vec3(0,100,0);
	float rot = 0.0;    // degrees 
	glm::vec3 scale = glm::vec3(1, 1, 1);

	float radius = 20.0;
	float width = 20.0;
	float height = 20.0;
};

/* A Shape that handles movements with physics */
class DynamicShape : public Shape {
public:
	DynamicShape() {
		// constants 
		damping = .99;
		mass = 50;

		// linear 
		speed = 0;
		velocity.set(0, 0, 0);
		acceleration.set(0, 0, 0);
		forces.set(0, 0, 0);

		// angular
		torque = 0;
		angVelocity = 0;
		angAcceleration = 0;
		rotForces = 0;
	}

	/* Update position, velocity, and acceleration with Euler's method */
	void integrate() {

		/* Gravity */
		forces += gravityForce();

		/* Linear integration */

		// interval for this step
		float dt = 1.0 / ofGetFrameRate();

		// update position based on velocity
		pos += (velocity * dt);

		// update acceleration with accumulated particles forces
		glm::vec3 accel = acceleration;    // start with any acceleration already on the particle
		accel += (forces * (1.0 / mass));
		velocity += accel * dt; // constant for faster decelerations 0.3

		// add damping
		velocity *= damping; // constant for faster decelerations 0.8

		/* Angular integration */

		// update rotation based on velocity
		rot += (angVelocity * dt);

		// update angular acceleration with accumulated particle forces
		float rotAccel = angAcceleration;  // start with any angular acceleration already on the particle
		rotAccel += (rotForces * (1.0 / mass));
		angVelocity += rotAccel * dt;

		// add damping
		angVelocity *= damping;  // constant for faster decelerations 0.8

		// clear forces on particle (they get re-added each step)
		forces.set(0, 0, 0);
		rotForces = 0;

	}

	/* Calculate heading vector by rotating "forward" vector to current angle of shape */
	glm::vec3 heading() {
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0), glm::radians(rot), glm::vec3(0, 1, 0));
		return glm::normalize(rotation * glm::vec4(glm::vec3(0, 0, 1), 0)); // try 0 vs 1 for last parameter
	}

	/* Calcuate the thrust force that propells the player forward in the direction of the heading vector */
	glm::vec3 thrustForce() {
		// thrustForce = speed * ||heading||
		return speed * glm::normalize(heading());
	}

	/* Calculate the lateral force to rotate the player */
	glm::vec3 lateralForce() {
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0), glm::radians(-90.0f), glm::vec3(0, 1, 0));
		glm::vec3 rightDirection = glm::normalize(rotation * glm::vec4(heading(), 0));
		return speed * rightDirection;
	}

	/* Return the lift force that propells the player upward */
	glm::vec3 liftForce() {
		return glm::vec3(0, speed, 0);
	}

	/* Calculate the gravity force that pulls the player downward */
	glm::vec3 gravityForce() {
		return glm::vec3(0, -1.62f, 0);
	}

	/* Rotate player counterclockwise */
	void rotateCounterclockwise() {
		rotForces += torque;
	}

	/* Rotate player clockwise */
	void rotateClockwise() {
		rotForces -= torque;
	}

	/* Move the player upward */
	void moveUp() {
		forces += liftForce();
	}

	/* Move the player downward */
	void moveDown() {
		forces += -liftForce();
	}

	/* Propel the player forward along the heading vector */
	void moveForward() {
		forces += thrustForce(); // * 0.5
	}

	/* Propel the player backward along the heading vector */
	void moveBackward() {
		forces += -thrustForce();
	}
	/* Propel the player right of the heading vector */
	void moveRight() {
		forces += lateralForce();
	}
	/* Propel the player left of the heading vector */
	void moveLeft() {
		forces -= lateralForce();
	}

	float damping;
	float mass;

	float speed;
	ofVec3f velocity;
	ofVec3f acceleration;
	ofVec3f forces;

	float torque;
	float angVelocity;
	float angAcceleration;
	float rotForces;
};



