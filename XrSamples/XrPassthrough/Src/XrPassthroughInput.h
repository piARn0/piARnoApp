#pragma once

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#define XR_USE_GRAPHICS_API_OPENGL_ES 1
#define XR_USE_PLATFORM_ANDROID 1
#include <openxr/openxr.h>
#include <openxr/openxr_oculus.h>
#include <openxr/openxr_oculus_helpers.h>
#include <openxr/openxr_platform.h>

void AppInput_init(App& app);
void AppInput_shutdown();
void AppInput_syncActions(App& app);

extern XrActionStateBoolean leftTriggerState;
extern XrActionStateBoolean rightTriggerState;

extern XrActionStateBoolean leftSqueezeState;
extern XrActionStateBoolean rightSqueezeState;

extern XrActionStateBoolean xButtonPressState;
extern XrActionStateBoolean yButtonPressState;

extern XrActionStateBoolean aButtonPressState;
extern XrActionStateBoolean bButtonPressState;

extern XrActionStateFloat rightTriggerHoldState;

extern bool leftControllerActive;
extern bool rightControllerActive;

extern XrSpace leftControllerAimSpace;
extern XrSpace rightControllerAimSpace;
extern XrSpace leftControllerGripSpace;
extern XrSpace rightControllerGripSpace;
