//
// Created by JW on 25/06/2022.
//

#include "Engine.h"
#include <vector>
#include <openxr/openxr.h>

using namespace OVR;


Engine::Engine(Scene *scene) : scene(scene) {
    piarno.init(this);
}

OVR::Posef Engine::getControllerPose(int index) {
    return scene->trackedController[index].pose;
}

Geometry* Engine::getGeometry(Mesh mesh) {
    return &scene->geometries[static_cast<size_t>(mesh)];
}

void Engine::update() {
    //TODO: update controller pos, update system timer, etc...
    piarno.update();
}

void Engine::render() {
    piarno.render();

    float x = -2, y = 0, z = -1;
    for (auto &g: scene->geometries) {
        g.render(OVR::Matrix4f::Translation(x, y, z) * OVR::Matrix4f::Scaling(0.1));
        x += 1;
    }
}

std::vector<Geometry> Engine::load_geometries() {
    std::vector<Geometry> g;

    //TODO: RN THIS REQUIRES U TO LOAD AND PUSH OBJS IN THE EXACT SAME ORDER AS DEFINED IN THE ENUM
    {
#include "models/axes.h"
        g.emplace_back(std::move(vertices), std::move(colors), std::move(indices));
    }

    {
#include "models/cube.h"
        g.emplace_back(std::move(vertices), std::move(colors), std::move(indices));
    }

    {
#include "models/rect.h"
        g.emplace_back(std::move(vertices), std::move(colors), std::move(indices));
    }

//    {
//#include "models/line.h"
//        g.push_back(Geometry(std::move(vertices), std::move(colors), std::move(indices)));
//    }

    {
        #include "models/teapot.h"

        for(auto &v : vertices)
            v /= 100.0;
        std::vector<unsigned char> colors(vertices.size() * 4 / 3, 255);

        g.emplace_back(std::move(vertices), std::move(colors), std::move(indices));
    }
    {
        #include "models/alphabet.h"
        std::vector<unsigned char> colors(vertices.size() * 4 / 3, 255);

        g.emplace_back(std::move(vertices), std::move(colors), std::move(indices));
    }

    return g;
}

// FIXME: WTF is CHECK_XRCMD ?
// TODO: this is inspired by https://github.com/KhronosGroup/OpenXR-SDK-Source/blob/main/src/tests/hello_xr/openxr_program.cpp
void Engine::initializeActions() {
// Create an action set.
    {
        XrActionSetCreateInfo actionSetInfo{XR_TYPE_ACTION_SET_CREATE_INFO};
        strcpy(actionSetInfo.actionSetName, "gameplay");
        strcpy(actionSetInfo.localizedActionSetName, "Gameplay");
        actionSetInfo.priority = 0;
        CHECK_XRCMD(xrCreateActionSet(m_instance, &actionSetInfo, &m_input.actionSet));
    }

    // Get the XrPath for the left and right hands - we will use them as subaction paths.
    CHECK_XRCMD(xrStringToPath(m_instance, "/user/hand/left", &m_input.handSubactionPath[Side::LEFT]));
    CHECK_XRCMD(xrStringToPath(m_instance, "/user/hand/right", &m_input.handSubactionPath[Side::RIGHT]));

    // Create actions.
    {
// Create an input action for grabbing objects with the left and right hands.
        XrActionCreateInfo actionInfo{XR_TYPE_ACTION_CREATE_INFO};
        actionInfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
        strcpy(actionInfo.actionName, "grab_object");
        strcpy(actionInfo.localizedActionName, "Grab Object");
        actionInfo.countSubactionPaths = uint32_t(m_input.handSubactionPath.size());
        actionInfo.subactionPaths = m_input.handSubactionPath.data();
        CHECK_XRCMD(xrCreateAction(m_input.actionSet, &actionInfo, &m_input.grabAction));
// Create an input action getting the left and right hand poses.
        actionInfo.actionType = XR_ACTION_TYPE_POSE_INPUT;
        strcpy(actionInfo.actionName, "hand_pose");
        strcpy(actionInfo.localizedActionName, "Hand Pose");
        actionInfo.countSubactionPaths = uint32_t(m_input.handSubactionPath.size());
        actionInfo.subactionPaths = m_input.handSubactionPath.data();
        CHECK_XRCMD(xrCreateAction(m_input.actionSet, &actionInfo, &m_input.poseAction));
// Create output actions for vibrating the left and right controller.
        actionInfo.actionType = XR_ACTION_TYPE_VIBRATION_OUTPUT;
        strcpy(actionInfo.actionName, "vibrate_hand");
        strcpy(actionInfo.localizedActionName, "Vibrate Hand");
        actionInfo.countSubactionPaths = uint32_t(m_input.handSubactionPath.size());
        actionInfo.subactionPaths = m_input.handSubactionPath.data();
        CHECK_XRCMD(xrCreateAction(m_input.actionSet, &actionInfo, &m_input.vibrateAction));
// Create input actions for quitting the session using the left and right controller.
// Since it doesn't matter which hand did this, we do not specify subaction paths for it.
// We will just suggest bindings for both hands, where possible.
        actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        strcpy(actionInfo.actionName, "quit_session");
        strcpy(actionInfo.localizedActionName, "Quit Session");
        actionInfo.countSubactionPaths = 0;
        actionInfo.subactionPaths = nullptr;
        CHECK_XRCMD(xrCreateAction(m_input.actionSet, &actionInfo, &m_input.quitAction));
    }
    std::array<XrPath, Side::COUNT> selectPath;
    std::array<XrPath, Side::COUNT> squeezeValuePath;
    std::array<XrPath, Side::COUNT> squeezeForcePath;
    std::array<XrPath, Side::COUNT> squeezeClickPath;
    std::array<XrPath, Side::COUNT> posePath;
    std::array<XrPath, Side::COUNT> hapticPath;
    std::array<XrPath, Side::COUNT> menuClickPath;
    std::array<XrPath, Side::COUNT> bClickPath;
    std::array<XrPath, Side::COUNT> triggerValuePath;
    CHECK_XRCMD(xrStringToPath(m_instance, "/user/hand/left/input/select/click", &selectPath[Side::LEFT]));
    CHECK_XRCMD(xrStringToPath(m_instance, "/user/hand/right/input/select/click", &selectPath[Side::RIGHT]));
    CHECK_XRCMD(xrStringToPath(m_instance, "/user/hand/left/input/squeeze/value", &squeezeValuePath[Side::LEFT]));
    CHECK_XRCMD(xrStringToPath(m_instance, "/user/hand/right/input/squeeze/value", &squeezeValuePath[Side::RIGHT]));
    CHECK_XRCMD(xrStringToPath(m_instance, "/user/hand/left/input/squeeze/force", &squeezeForcePath[Side::LEFT]));
    CHECK_XRCMD(xrStringToPath(m_instance, "/user/hand/right/input/squeeze/force", &squeezeForcePath[Side::RIGHT]));
    CHECK_XRCMD(xrStringToPath(m_instance, "/user/hand/left/input/squeeze/click", &squeezeClickPath[Side::LEFT]));
    CHECK_XRCMD(xrStringToPath(m_instance, "/user/hand/right/input/squeeze/click", &squeezeClickPath[Side::RIGHT]));
    CHECK_XRCMD(xrStringToPath(m_instance, "/user/hand/left/input/grip/pose", &posePath[Side::LEFT]));
    CHECK_XRCMD(xrStringToPath(m_instance, "/user/hand/right/input/grip/pose", &posePath[Side::RIGHT]));
    CHECK_XRCMD(xrStringToPath(m_instance, "/user/hand/left/output/haptic", &hapticPath[Side::LEFT]));
    CHECK_XRCMD(xrStringToPath(m_instance, "/user/hand/right/output/haptic", &hapticPath[Side::RIGHT]));
    CHECK_XRCMD(xrStringToPath(m_instance, "/user/hand/left/input/menu/click", &menuClickPath[Side::LEFT]));
    CHECK_XRCMD(xrStringToPath(m_instance, "/user/hand/right/input/menu/click", &menuClickPath[Side::RIGHT]));
    CHECK_XRCMD(xrStringToPath(m_instance, "/user/hand/left/input/b/click", &bClickPath[Side::LEFT]));
    CHECK_XRCMD(xrStringToPath(m_instance, "/user/hand/right/input/b/click", &bClickPath[Side::RIGHT]));
    CHECK_XRCMD(xrStringToPath(m_instance, "/user/hand/left/input/trigger/value", &triggerValuePath[Side::LEFT]));
    CHECK_XRCMD(xrStringToPath(m_instance, "/user/hand/right/input/trigger/value", &triggerValuePath[Side::RIGHT]));

    // Suggest bindings for the Oculus Touch.
    {
        XrPath oculusTouchInteractionProfilePath;
        CHECK_XRCMD(
                xrStringToPath(m_instance, "/interaction_profiles/oculus/touch_controller", &oculusTouchInteractionProfilePath));
        std::vector<XrActionSuggestedBinding> bindings{{{m_input.grabAction, squeezeValuePath[Side::LEFT]},
                                                        {m_input.grabAction, squeezeValuePath[Side::RIGHT]},
                                                        {m_input.poseAction, posePath[Side::LEFT]},
                                                        {m_input.poseAction, posePath[Side::RIGHT]},
                                                        {m_input.quitAction, menuClickPath[Side::LEFT]},
                                                        {m_input.vibrateAction, hapticPath[Side::LEFT]},
                                                        {m_input.vibrateAction, hapticPath[Side::RIGHT]}}};
        XrInteractionProfileSuggestedBinding suggestedBindings{XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
        suggestedBindings.interactionProfile = oculusTouchInteractionProfilePath;
        suggestedBindings.suggestedBindings = bindings.data();
        suggestedBindings.countSuggestedBindings = (uint32_t)bindings.size();
        CHECK_XRCMD(xrSuggestInteractionProfileBindings(m_instance, &suggestedBindings));
    }

    XrActionSpaceCreateInfo actionSpaceInfo{XR_TYPE_ACTION_SPACE_CREATE_INFO};
    actionSpaceInfo.action = m_input.poseAction;
    actionSpaceInfo.poseInActionSpace.orientation.w = 1.f;
    actionSpaceInfo.subactionPath = m_input.handSubactionPath[Side::LEFT];
    CHECK_XRCMD(xrCreateActionSpace(m_session, &actionSpaceInfo, &m_input.handSpace[Side::LEFT]));
    actionSpaceInfo.subactionPath = m_input.handSubactionPath[Side::RIGHT];
    CHECK_XRCMD(xrCreateActionSpace(m_session, &actionSpaceInfo, &m_input.handSpace[Side::RIGHT]));
    XrSessionActionSetsAttachInfo attachInfo{XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO};
    attachInfo.countActionSets = 1;
    attachInfo.actionSets = &m_input.actionSet;
    CHECK_XRCMD(xrAttachSessionActionSets(m_session, &attachInfo));

}

