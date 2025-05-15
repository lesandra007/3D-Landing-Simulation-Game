#pragma once
#include "ofMain.h"

class CameraSystem {
public:
    enum CameraMode {
        TRACKING_CAMERA,
        ONBOARD_CAMERA,
        EASYCAM
    };

    CameraSystem() {
        currentMode = EASYCAM;

        // Initialize EasyCam
        easyCam.setDistance(90);
        easyCam.setNearClip(0.1);
        easyCam.setFov(65.5);

        // Initialize tracking camera
        trackingCamPos = glm::vec3(30, 30, 30);
        trackingCam.setPosition(trackingCamPos);
        trackingCam.setNearClip(0.1);
        trackingCam.setFov(65.5);

        // Initialize onboard camera
        onboardCamPos = glm::vec3(0, 0, 0);
        onboardCamOffset = glm::vec3(0, 2, 0); // Position slightly above the lander
        onboardCamDirection = glm::vec3(0, 0, -1); // Looking forward
        onboardCam.setPosition(onboardCamPos);
        onboardCam.setNearClip(0.1);
        onboardCam.setFov(85); // Wider FOV for onboard camera
    }

    void setup() {
        easyCam.disableMouseInput();
    }

    void setCameraTarget(const glm::vec3& target) {
        targetPosition = target;

        // Set tracking camera to look at target
        trackingCam.lookAt(targetPosition);

        // If in EasyCam mode, set the target
        if (currentMode == EASYCAM) {
            easyCam.setTarget(targetPosition);
        }
    }

    void updateOnboardCamera(const glm::vec3& playerPos, const glm::vec3& playerForward, const glm::vec3& playerUp) {
        // Position the camera on the lander with the given offset
        onboardCamPos = playerPos + onboardCamOffset;

        // Look in the direction specified by onboardCamDirection, transformed based on player orientation
        glm::vec3 lookDir = playerForward;

        onboardCam.setPosition(onboardCamPos);
        onboardCam.lookAt(onboardCamPos + lookDir, playerUp);
    }

    void begin() {
        switch (currentMode) {
        case TRACKING_CAMERA:
            trackingCam.begin();
            break;
        case ONBOARD_CAMERA:
            onboardCam.begin();
            break;
        case EASYCAM:
            easyCam.begin();
            break;
        }
    }

    void end() {
        switch (currentMode) {
        case TRACKING_CAMERA:
            trackingCam.end();
            break;
        case ONBOARD_CAMERA:
            onboardCam.end();
            break;
        case EASYCAM:
            easyCam.end();
            break;
        }
    }

    void switchCameraMode(CameraMode mode) {
        currentMode = mode;

        // If switching to EasyCam, restore its target
        if (currentMode == EASYCAM) {
            easyCam.setTarget(targetPosition);
        }
    }

    void enableMouseInput() {
        if (currentMode == EASYCAM) {
            easyCam.enableMouseInput();
        }
    }

    void disableMouseInput() {
        if (currentMode == EASYCAM) {
            easyCam.disableMouseInput();
        }
    }

    bool getMouseInputEnabled() {
        if (currentMode == EASYCAM) {
            return easyCam.getMouseInputEnabled();
        }
    }

    void setOnboardCameraDirection(const glm::vec3& direction) {
        onboardCamDirection = direction;
    }

    void reset() {
        currentMode = EASYCAM;

        easyCam.setDistance(60);
        easyCam.setNearClip(0.1);
        easyCam.setFov(65.5);

        trackingCamPos = glm::vec3(30, 30, 30);
        trackingCam.setPosition(trackingCamPos);
        trackingCam.setNearClip(0.1);
        trackingCam.setFov(65.5);

        onboardCamDirection = glm::vec3(0, 0, -1); // Looking forward
        onboardCam.setPosition(onboardCamPos);
        onboardCam.setNearClip(0.1);
        onboardCam.setFov(85); // Wider FOV for onboard camera
    }

    CameraMode getCurrentMode() const {
        return currentMode;
    }

    ofEasyCam& getEasyCam() {
        return easyCam;
    }

public:
    CameraMode currentMode;
    glm::vec3 targetPosition;

    // EasyCam (free camera)
    ofEasyCam easyCam;

    // Tracking camera (fixed position, looking at spaceship)
    ofCamera trackingCam;
    glm::vec3 trackingCamPos;

    // Onboard camera (attached to spaceship)
    ofCamera onboardCam;
    glm::vec3 onboardCamPos;
    glm::vec3 onboardCamOffset;
    glm::vec3 onboardCamDirection;
};