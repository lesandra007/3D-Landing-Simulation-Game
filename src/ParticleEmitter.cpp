
//  Kevin M. Smith - CS 134 SJSU

#include "ParticleEmitter.h"

ParticleEmitter::ParticleEmitter() {
	sys = new ParticleSystem();
	createdSys = true;
	init();
}

ParticleEmitter::ParticleEmitter(ParticleSystem *s) {
	if (s == NULL)
	{
		cout << "fatal error: null particle system passed to ParticleEmitter()" << endl;
		ofExit();
	}
	sys = s;
	createdSys = false;
	init();
}

ParticleEmitter::~ParticleEmitter() {

	// deallocate particle system if emitter created one internally
	//
	if (createdSys) delete sys;
}

void ParticleEmitter::init() {
	rate = 1;
	particleVelocity = ofVec3f(0, 5, 0);
	lifespan = 3;
	started = false;
	oneShot = false;
	fired = false;
	lastSpawned = 0;
	radius = 1;
	particleRadius = 1;
	visible = true;
	type = DirectionalEmitter;
	groupSize = 1;
}



void ParticleEmitter::draw() {
	if (visible) {
		switch (type) {
		case DirectionalEmitter:
			ofDrawSphere(pos, radius/10);  // just draw a small sphere for point emitters 
			break;
		case SphereEmitter:
		case DiskEmitter:
			ofDrawSphere(pos, radius / 2);
			break;
		case RadialEmitter:
			ofDrawSphere(pos, radius/10);  // just draw a small sphere as a placeholder
			break;
		default:
			break;
		}
	}
	sys->draw();  
}
void ParticleEmitter::start() {
	started = true;
	lastSpawned = ofGetElapsedTimeMillis();
}

void ParticleEmitter::stop() {
	started = false;
	fired = false;
}
void ParticleEmitter::update() {

	float time = ofGetElapsedTimeMillis();
	if (oneShot && started) {
		if (!fired) {
			// spawn a new particle(s)
			//
			for (int i = 0; i < groupSize; i++)
				spawn(time);

			lastSpawned = time;
		}
		fired = true;
		stop();
	}

	else if (((time - lastSpawned) > (1000.0 / rate)) && started) {

		// spawn a new particle(s)
		//
		for (int i= 0; i < groupSize; i++)
			spawn(time);
	
		lastSpawned = time;
	}

	sys->update();
}

// spawn a single particle.  time is current time of birth
//
void ParticleEmitter::spawn(float time) {

	Particle particle;

	// set initial velocity and position
	// based on emitter type
	//
	switch (type) {
	case RadialEmitter:
	{
		ofVec3f dir = ofVec3f(ofRandom(-1, 1), ofRandom(-1, 1), ofRandom(-1, 1));
		float speed = particleVelocity.length() * 0.1;
		particle.velocity = dir.getNormalized() * speed;
		particle.position.set(pos);
	}
	break;
	case SphereEmitter:
		break;
	case DiskEmitter: 
	{
		float speed = particleVelocity.length() * 0.1;
		particle.velocity = ofVec3f(0, -speed, 0);

		// emit from a disk shape instead of a singular point
		float angle = ofRandom(0, TWO_PI); // Random angle in a full circle
		float distance = ofRandom(0, radius); // Random distance from center
		float x = cos(angle) * distance;
		float z = sin(angle) * distance;

		particle.position.set(ofVec3f(x, pos.y, z)); // Ensures a circular area
		break;
	}
	case DirectionalEmitter:
		particle.velocity = particleVelocity;
		particle.position.set(pos);
		break;
	}

	// other particle attributes
	//
	particle.lifespan = lifespan;
	particle.birthtime = time;
	particle.radius = particleRadius;

	// add to system
	//
	sys->add(particle);
}
