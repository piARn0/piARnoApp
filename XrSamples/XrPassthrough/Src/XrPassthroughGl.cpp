/************************************************************************************

Filename	:	XrPassthroughGl.cpp
Content		:	This sample is derived from MagicCarpetXr.
                When used in room scale mode, it draws a "carpet" under the
                user to indicate where it is safe to walk around.
Created		:	November, 2018
Authors		:	Cass Everitt

Copyright	:	Copyright (c) Facebook Technologies, LLC and its affiliates. All rights reserved.

*************************************************************************************/

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <vector>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h> // for prctl( PR_SET_NAME )
#include <android/log.h>
#include <android/native_window_jni.h> // for native window JNI
#include <android/input.h>

#include <atomic>
#include <thread>

#include <sys/system_properties.h>

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include "XrPassthroughGl.h"
#include "Engine.h"

using namespace OVR;

// EXT_texture_border_clamp
#ifndef GL_CLAMP_TO_BORDER
#define GL_CLAMP_TO_BORDER 0x812D
#endif

#ifndef GL_TEXTURE_BORDER_COLOR
#define GL_TEXTURE_BORDER_COLOR 0x1004
#endif

#ifndef GL_FRAMEBUFFER_SRGB_EXT
#define GL_FRAMEBUFFER_SRGB_EXT 0x8DB9
#endif

#if !defined(GL_EXT_multisampled_render_to_texture)

typedef void(GL_APIENTRY *PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)(
        GLenum target,
        GLsizei samples,
        GLenum internalformat,
        GLsizei width,
        GLsizei height);

typedef void(GL_APIENTRY *PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC)(
        GLenum target,
        GLenum attachment,
        GLenum textarget,
        GLuint texture,
        GLint level,
        GLsizei samples);

#endif

#if !defined(GL_OVR_multiview)

typedef void(GL_APIENTRY *PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC)(
        GLenum target,
        GLenum attachment,
        GLuint texture,
        GLint level,
        GLint baseViewIndex,
        GLsizei numViews);

#endif

#if !defined(GL_OVR_multiview_multisampled_render_to_texture)

typedef void(GL_APIENTRY *PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVRPROC)(
        GLenum target,
        GLenum attachment,
        GLuint texture,
        GLint level,
        GLsizei samples,
        GLint baseViewIndex,
        GLsizei numViews);

#endif

#define DEBUG 1
#define OVR_LOG_TAG "XrPassthroughGl"

#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, OVR_LOG_TAG, __VA_ARGS__)
#if DEBUG
#define ALOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, OVR_LOG_TAG, __VA_ARGS__)
#else
#define ALOGV(...)
#endif

/*
================================================================================

OpenGL-ES Utility Functions

================================================================================
*/

namespace {
    struct OpenGLExtensions_t {
        bool multi_view; // GL_OVR_multiview, GL_OVR_multiview2
        bool EXT_texture_border_clamp; // GL_EXT_texture_border_clamp, GL_OES_texture_border_clamp
        bool EXT_sRGB_write_control;
    };

    OpenGLExtensions_t glExtensions;
} // namespace

static void EglInitExtensions() {
    glExtensions = {};
    const char *allExtensions = (const char *) glGetString(GL_EXTENSIONS);
    if (allExtensions != nullptr) {
        glExtensions.multi_view = strstr(allExtensions, "GL_OVR_multiview2") &&
                                  strstr(allExtensions,
                                         "GL_OVR_multiview_multisampled_render_to_texture");

        glExtensions.EXT_texture_border_clamp =
                strstr(allExtensions, "GL_EXT_texture_border_clamp") ||
                strstr(allExtensions, "GL_OES_texture_border_clamp");
        glExtensions.EXT_sRGB_write_control = strstr(allExtensions, "GL_EXT_sRGB_write_control");
    }
}

static const char *GlFrameBufferStatusString(GLenum status) {
    switch (status) {
        case GL_FRAMEBUFFER_UNDEFINED:
            return "GL_FRAMEBUFFER_UNDEFINED";
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
        case GL_FRAMEBUFFER_UNSUPPORTED:
            return "GL_FRAMEBUFFER_UNSUPPORTED";
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
        default:
            return "unknown";
    }
}

#ifdef CHECK_GL_ERRORS

static const char* GlErrorString(GLenum error) {
    switch (error) {
        case GL_NO_ERROR:
            return "GL_NO_ERROR";
        case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:
            return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";
        case GL_OUT_OF_MEMORY:
            return "GL_OUT_OF_MEMORY";
        default:
            return "unknown";
    }
}

static void GLCheckErrors(int line) {
    for (int i = 0; i < 10; i++) {
        const GLenum error = glGetError();
        if (error == GL_NO_ERROR) {
            break;
        }
        ALOGE("GL error on line %d: %s", line, GlErrorString(error));
    }
}

#define GL(func) \
    func;        \
    GLCheckErrors(__LINE__);

#else // CHECK_GL_ERRORS

#define GL(func) func;

#endif // CHECK_GL_ERRORS



// -------

enum VertexAttributeLocation {
    VERTEX_ATTRIBUTE_LOCATION_POSITION,
    VERTEX_ATTRIBUTE_LOCATION_COLOR,
    VERTEX_ATTRIBUTE_LOCATION_UV,
    VERTEX_ATTRIBUTE_LOCATION_TRANSFORM
};

struct VertexAttribute {
    enum VertexAttributeLocation location;
    const char *name;
};

static VertexAttribute ProgramVertexAttributes[] = {
        {VERTEX_ATTRIBUTE_LOCATION_POSITION,  "vertexPosition"},
        {VERTEX_ATTRIBUTE_LOCATION_COLOR,     "vertexColor"},
        {VERTEX_ATTRIBUTE_LOCATION_UV,        "vertexUv"},
        {VERTEX_ATTRIBUTE_LOCATION_TRANSFORM, "vertexTransform"}};


/*
================================================================================

Program

================================================================================
*/

struct Uniform {
    enum Index {
        MODEL_MATRIX,
        VIEW_ID,
        SCENE_MATRICES,
    };
    enum Type {
        VECTOR4,
        MATRIX4X4,
        INTEGER,
        BUFFER,
    };

    Index index;
    Type type;
    const char *name;
};

static Uniform ProgramUniforms[] = {
        {Uniform::Index::MODEL_MATRIX,   Uniform::Type::MATRIX4X4, "ModelMatrix"},
        {Uniform::Index::VIEW_ID,        Uniform::Type::INTEGER,   "ViewID"},
        {Uniform::Index::SCENE_MATRICES, Uniform::Type::BUFFER,    "sceneMatrices"},
};

void Program::clear() {
    program = 0;
    vertexShader = 0;
    fragmentShader = 0;
    memset(uniformLocation, 0, sizeof(uniformLocation));
    memset(uniformBinding, 0, sizeof(uniformBinding));
    memset(textures, 0, sizeof(textures));
}

static const char *programVersion = "#version 300 es\n";

bool Program::create(const char *vertexSource, const char *fragmentSource) {
    GLint r;

    GL(vertexShader = glCreateShader(GL_VERTEX_SHADER));

    const char *vertexSources[3] = {programVersion, "", vertexSource};
    GL(glShaderSource(vertexShader, 3, vertexSources, 0));
    GL(glCompileShader(vertexShader));
    GL(glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &r));
    if (r == GL_FALSE) {
        GLchar msg[4096];
        GL(glGetShaderInfoLog(vertexShader, sizeof(msg), 0, msg));
        ALOGE("%s\n%s\n", vertexSource, msg);
        return false;
    }

    const char *fragmentSources[2] = {programVersion, fragmentSource};
    GL(fragmentShader = glCreateShader(GL_FRAGMENT_SHADER));
    GL(glShaderSource(fragmentShader, 2, fragmentSources, 0));
    GL(glCompileShader(fragmentShader));
    GL(glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &r));
    if (r == GL_FALSE) {
        GLchar msg[4096];
        GL(glGetShaderInfoLog(fragmentShader, sizeof(msg), 0, msg));
        ALOGE("%s\n%s\n", fragmentSource, msg);
        return false;
    }

    GL(program = glCreateProgram());
    GL(glAttachShader(program, vertexShader));
    GL(glAttachShader(program, fragmentShader));

    // bind the vertex attribute locations.
    for (size_t i = 0; i < sizeof(ProgramVertexAttributes) / sizeof(ProgramVertexAttributes[0]);
         i++) {
        GL(glBindAttribLocation(
                program, ProgramVertexAttributes[i].location, ProgramVertexAttributes[i].name));
    }

    GL(glLinkProgram(program));
    GL(glGetProgramiv(program, GL_LINK_STATUS, &r));
    if (r == GL_FALSE) {
        GLchar msg[4096];
        GL(glGetProgramInfoLog(program, sizeof(msg), 0, msg));
        ALOGE("Linking program failed: %s\n", msg);
        return false;
    }

    int numBufferBindings = 0;

    memset(uniformLocation, -1, sizeof(uniformLocation));
    for (size_t i = 0; i < sizeof(ProgramUniforms) / sizeof(ProgramUniforms[0]); i++) {
        const int uniformIndex = ProgramUniforms[i].index;
        if (ProgramUniforms[i].type == Uniform::Type::BUFFER) {
            GL(uniformLocation[uniformIndex] =
                       glGetUniformBlockIndex(program, ProgramUniforms[i].name));
            uniformBinding[uniformIndex] = numBufferBindings++;
            GL(glUniformBlockBinding(
                    program, uniformLocation[uniformIndex], uniformBinding[uniformIndex]));
        } else {
            GL(uniformLocation[uniformIndex] =
                       glGetUniformLocation(program, ProgramUniforms[i].name));
            uniformBinding[uniformIndex] = uniformLocation[uniformIndex];
        }
    }

    GL(glUseProgram(program));

    // Get the texture locations.
    for (int i = 0; i < MAX_PROGRAM_TEXTURES; i++) {
        char name[32];
        sprintf(name, "Texture%i", i);
        textures[i] = glGetUniformLocation(program, name);
        if (textures[i] != -1) {
            GL(glUniform1i(textures[i], i));
        }
    }

    GL(glUseProgram(0));

    return true;
}

void Program::destroy() {
    if (program != 0) {
        GL(glDeleteProgram(program));
        program = 0;
    }
    if (vertexShader != 0) {
        GL(glDeleteShader(vertexShader));
        vertexShader = 0;
    }
    if (fragmentShader != 0) {
        GL(glDeleteShader(fragmentShader));
        fragmentShader = 0;
    }
}

static const char VERTEX_SHADER[] =
        "#define NUM_VIEWS 2\n"
        "#define VIEW_ID gl_ViewID_OVR\n"
        "#extension GL_OVR_multiview2 : require\n"
        "layout(num_views=NUM_VIEWS) in;\n"
        "in vec3 vertexPosition;\n"
        "in vec4 vertexColor;\n"
        "uniform mat4 ModelMatrix;\n"
        "uniform sceneMatrices\n"
        "{\n"
        "   uniform mat4 ViewMatrix[NUM_VIEWS];\n"
        "   uniform mat4 ProjectionMatrix[NUM_VIEWS];\n"
        "} sm;\n"
        "out vec4 fragmentColor;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = sm.ProjectionMatrix[VIEW_ID] * ( sm.ViewMatrix[VIEW_ID] * ( ModelMatrix * ( vec4( vertexPosition, 1.0 ) ) ) );\n"
        "   fragmentColor = vertexColor;\n"
        "}\n";

static const char FRAGMENT_SHADER[] =
        "in lowp vec4 fragmentColor;\n"
        "out lowp vec4 outColor;\n"
        "void main()\n"
        "{\n"
        "   outColor = fragmentColor;\n"
        "}\n";

/*static const char CIRCLE_VERTEX_SHADER[] =
        "#define NUM_VIEWS 2\n"
        "#define VIEW_ID gl_ViewID_OVR\n"
        "#extension GL_OVR_multiview2 : require\n"
        "layout(num_views=NUM_VIEWS) in;\n"
        "in vec3 vertexPosition;\n"
        "in vec4 vertexColor;\n"
        "uniform mat4 ModelMatrix;\n"
        "uniform sceneMatrices\n"
        "{\n"
        "   uniform mat4 ViewMatrix[NUM_VIEWS];\n"
        "   uniform mat4 ProjectionMatrix[NUM_VIEWS];\n"
        "} sm;\n"
        "out highp vec3 fragPosEye;\n"
        "out highp vec3 circleCenter;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = sm.ProjectionMatrix[VIEW_ID] * ( sm.ViewMatrix[VIEW_ID] * ( ModelMatrix * ( vec4( vertexPosition, 1.0 ) ) ) );\n"
        "   fragPosEye = (sm.ViewMatrix[VIEW_ID] * ( ModelMatrix * ( vec4( vertexPosition, 1.0 ) ) ) ).xyz;\n"
        "   circleCenter = (sm.ViewMatrix[VIEW_ID] * ( ModelMatrix * ( vec4( 0.0, 0.0, 0.0, 1.0 ) ) ) ).xyz;\n"
        "}\n";

static const char CIRCLE_FRAGMENT_SHADER[] =
        "in highp vec3 fragPosEye;\n"
        "in highp vec3 circleCenter;\n"
        "out highp vec4 outColor;\n"
        "void main()\n"
        "{\n"
        "   highp vec3 dir = fragPosEye / length(fragPosEye);\n"
        "   highp float a = dot(dir, dir);\n"
        "   highp float b = 2.0 * dot(dir, -circleCenter);\n"
        "   highp float r = 0.2;\n"
        "   highp float c = dot(circleCenter, circleCenter) - r * r;\n"
        "   highp float discr = b * b - 4.0 * a * c;\n"
        "   if (discr < 0.0) { discard; }\n"
        "   highp float t = (-b - sqrt(discr)) / (2.0 * a);\n"
        "   highp vec3 n = (t * dir) - circleCenter;\n"
        "   n = n / length(n);\n"
        "   highp float ndote = dot(n, -dir);\n"
        "   highp float alpha = 0.5 + 0.5 * smoothstep(0.35, 0.5, 1.0 - ndote);\n"
        "   outColor = vec4(1.0, 0.0, 0.0, alpha);\n"
        "}\n";
*/

/*
================================================================================

Geometry

================================================================================
*/

void Geometry::clear() {
    vertexBuffer = 0;
    colorBuffer = 0;
    indexBuffer = 0;
    vertexArrayObject = 0;
}

Geometry::Geometry(
        const std::vector<float> &vertexPositions,
        const std::vector<unsigned char> &colors,
        const std::vector<unsigned short> &indices,
        GLenum mode) : draw_mode(mode), global_color(false) {

    GL(glGenBuffers(1, &vertexBuffer));
    GL(glGenBuffers(1, &colorBuffer));
    GL(glGenBuffers(1, &indexBuffer));

    updateVertices(vertexPositions);
    updateColors(colors);
    updateIndices(indices);
}

Geometry::Geometry(const std::vector<float> &vertexPositions,
                   const std::vector<unsigned short> &indices,
                   GLenum mode) : draw_mode(mode), global_color(true) {
    GL(glGenBuffers(1, &vertexBuffer));
    GL(glGenBuffers(1, &indexBuffer));

    updateVertices(vertexPositions);
    updateIndices(indices);}

void Geometry::destroy() {
    GL(glDeleteBuffers(1, &indexBuffer));
    if(!global_color)
        GL(glDeleteBuffers(1, &colorBuffer));
    GL(glDeleteBuffers(1, &vertexBuffer));

    clear();
}

void Geometry::createVAO() {
    GL(glGenVertexArrays(1, &vertexArrayObject));
    GL(glBindVertexArray(vertexArrayObject));

    GL(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer));
    GL(glEnableVertexAttribArray(VERTEX_ATTRIBUTE_LOCATION_POSITION));
    GL(glVertexAttribPointer(
            VERTEX_ATTRIBUTE_LOCATION_POSITION, //index
            3, //size
            GL_FLOAT, //type
            false, //normalized
            3 * sizeof(float), //stride
            (const GLvoid *) 0)); //offset

    if(!global_color) {
        GL(glBindBuffer(GL_ARRAY_BUFFER, colorBuffer));
        GL(glEnableVertexAttribArray(VERTEX_ATTRIBUTE_LOCATION_COLOR));
        GL(glVertexAttribPointer(
                VERTEX_ATTRIBUTE_LOCATION_COLOR, //index
                4, //size
                GL_UNSIGNED_BYTE, //type
                true, //normalized
                4 * sizeof(unsigned char), //stride
                (const GLvoid *) 0)); //offset
    }

    GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer));

    GL(glBindVertexArray(0));
}

void Geometry::destroyVAO() {
    GL(glDeleteVertexArrays(1, &vertexArrayObject));
}

void Geometry::updateVertices(const std::vector<float> &vertexPositions) {
    GL(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer));
    GL(glBufferData(GL_ARRAY_BUFFER, vertexPositions.size() * sizeof(float), vertexPositions.data(),
                    GL_STATIC_DRAW));
    GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void Geometry::updateColors(const std::vector<unsigned char> &colors) {
    GL(glBindVertexArray(vertexArrayObject));
    GL(glBindBuffer(GL_ARRAY_BUFFER, colorBuffer));
    GL(glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(unsigned char), colors.data(),
                    GL_DYNAMIC_DRAW));
    GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void Geometry::updateIndices(const std::vector<unsigned short> &indices) {


    indexCount = indices.size();
    GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer));
    GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short),
                    indices.data(), GL_STATIC_DRAW));
    GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void Geometry::render(const Matrix4f &transform) {
    GL(glUseProgram(program->program));

    //set VAO for this obj
    GL(glBindVertexArray(vertexArrayObject));

    if (program->uniformLocation[Uniform::Index::VIEW_ID] >=
        0) { // NOTE: will not be present when multiview path is enabled.
        GL(glUniform1i(program->uniformLocation[Uniform::Index::VIEW_ID], 0));
    }

    //set transform
    glUniformMatrix4fv(program->uniformLocation[Uniform::Index::MODEL_MATRIX], 1,
                       GL_TRUE, &transform.M[0][0]);

    glDrawElements(draw_mode, indexCount, GL_UNSIGNED_SHORT, NULL);

    glBindVertexArray(0);
    glUseProgram(0);
}



/*
================================================================================

Framebuffer

================================================================================
*/

void Framebuffer::clear() {
    width = 0;
    height = 0;
    multisamples = 0;
    swapChainLength = 0;
    elements = nullptr;
}

bool Framebuffer::create(
        const GLenum colorFormat,
        const int _width,
        const int _height,
        const int _multisamples,
        const int _swapChainLength,
        GLuint *colorTextures) {
    PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC glFramebufferTextureMultiviewOVR =
            (PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC) eglGetProcAddress(
                    "glFramebufferTextureMultiviewOVR");
    PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVRPROC glFramebufferTextureMultisampleMultiviewOVR =
            (PFNGLFRAMEBUFFERTEXTUREMULTISAMPLEMULTIVIEWOVRPROC) eglGetProcAddress(
                    "glFramebufferTextureMultisampleMultiviewOVR");

    width = _width;
    height = _height;
    multisamples = _multisamples;
    swapChainLength = _swapChainLength;

    elements = new Element[swapChainLength];

    for (int i = 0; i < swapChainLength; i++) {
        Element &el = elements[i];
        // create the color buffer texture.
        el.colorTexture = colorTextures[i];
        GLenum colorTextureTarget = GL_TEXTURE_2D_ARRAY;
        GL(glBindTexture(colorTextureTarget, el.colorTexture));
        GL(glTexParameteri(colorTextureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
        GL(glTexParameteri(colorTextureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
        GLfloat borderColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
        GL(glTexParameterfv(colorTextureTarget, GL_TEXTURE_BORDER_COLOR, borderColor));
        GL(glTexParameteri(colorTextureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GL(glTexParameteri(colorTextureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GL(glBindTexture(colorTextureTarget, 0));

        // create the depth buffer texture.
        GL(glGenTextures(1, &el.depthTexture));
        GL(glBindTexture(GL_TEXTURE_2D_ARRAY, el.depthTexture));
        GL(glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT24, width, height, 2));
        GL(glBindTexture(GL_TEXTURE_2D_ARRAY, 0));

        // create the frame buffer.
        GL(glGenFramebuffers(1, &el.frameBufferObject));
        GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, el.frameBufferObject));
        if (multisamples > 1 && (glFramebufferTextureMultisampleMultiviewOVR != nullptr)) {
            GL(glFramebufferTextureMultisampleMultiviewOVR(
                    GL_DRAW_FRAMEBUFFER,
                    GL_DEPTH_ATTACHMENT,
                    el.depthTexture,
                    0 /* level */,
                    multisamples /* samples */,
                    0 /* baseViewIndex */,
                    2 /* numViews */));
            GL(glFramebufferTextureMultisampleMultiviewOVR(
                    GL_DRAW_FRAMEBUFFER,
                    GL_COLOR_ATTACHMENT0,
                    el.colorTexture,
                    0 /* level */,
                    multisamples /* samples */,
                    0 /* baseViewIndex */,
                    2 /* numViews */));
        } else {
            GL(glFramebufferTextureMultiviewOVR(
                    GL_DRAW_FRAMEBUFFER,
                    GL_DEPTH_ATTACHMENT,
                    el.depthTexture,
                    0 /* level */,
                    0 /* baseViewIndex */,
                    2 /* numViews */));
            GL(glFramebufferTextureMultiviewOVR(
                    GL_DRAW_FRAMEBUFFER,
                    GL_COLOR_ATTACHMENT0,
                    el.colorTexture,
                    0 /* level */,
                    0 /* baseViewIndex */,
                    2 /* numViews */));
        }

        GL(GLenum renderFramebufferStatus = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
        GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
        if (renderFramebufferStatus != GL_FRAMEBUFFER_COMPLETE) {
            ALOGE(
                    "Incomplete frame buffer object: %s",
                    GlFrameBufferStatusString(renderFramebufferStatus));
            return false;
        }
    }

    return true;
}

void Framebuffer::destroy() {
    for (int i = 0; i < swapChainLength; i++) {
        Element &el = elements[i];
        GL(glDeleteFramebuffers(1, &el.frameBufferObject));
        GL(glDeleteTextures(1, &el.depthTexture));
    }
    delete[] elements;
    clear();
}

void Framebuffer::bind(int element) {
    GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, elements[element].frameBufferObject));
}

void Framebuffer::unbind() {
    GL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
}

void Framebuffer::resolve() {
    // Discard the depth buffer, so the tiler won't need to write it back out to memory.
    const GLenum depthAttachment[1] = {GL_DEPTH_ATTACHMENT};
    glInvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, 1, depthAttachment);

    // We now let the resolve happen implicitly.
}

/*
================================================================================

TrackedController

================================================================================
*/

void TrackedController::clear() {
    active = false;
    pose = OVR::Posef::Identity();
}

/*
================================================================================

Scene

================================================================================
*/

void Scene::setClearColor(const float *c) {
    for (int i = 0; i < 4; i++) {
        clearColor[i] = c[i];
    }
}

void Scene::clear() {
    createdScene = false;
    createdVAOs = false;
    sceneMatrices = 0;

    for (auto &g: geometries)
        g.clear();
    program.clear();
}

bool Scene::isCreated() {
    return createdScene;
}

void Scene::createVAOs() {
    if (!createdVAOs) {
        for (auto &g: geometries)
            g.createVAO();
        createdVAOs = true;
    }
}

void Scene::destroyVAOs() {
    if (createdVAOs) {
        for (auto &g: geometries)
            g.destroyVAO();

        createdVAOs = false;
    }
}


void Scene::create() {
    // Setup the scene matrices.
    GL(glGenBuffers(1, &sceneMatrices));
    GL(glBindBuffer(GL_UNIFORM_BUFFER, sceneMatrices));
    GL(glBufferData(
            GL_UNIFORM_BUFFER,
            2 * sizeof(Matrix4f) /* 2 view matrices */ +
            2 * sizeof(Matrix4f) /* 2 projection matrices */,
            nullptr,
            GL_STATIC_DRAW));
    GL(glBindBuffer(GL_UNIFORM_BUFFER, 0));

    geometries = Engine::loadGeometries();

    createVAOs();

    // Generic program
    if (!program.create(VERTEX_SHADER, FRAGMENT_SHADER)) {
        ALOGE("Failed to compile generic program");
    }

    for (auto &g: geometries)
        g.program = &program;

    createdScene = true;

    float c[] = {0.3, 0.3, 0.3, 0.0};
    setClearColor(c);
}

void Scene::destroy() {
    destroyVAOs();
    GL(glDeleteBuffers(1, &sceneMatrices));

    for (auto &g: geometries)
        g.destroy();
    program.destroy();
    createdScene = false;
}

/*
================================================================================

AppRenderer

================================================================================
*/

void AppRenderer::clear() {
    framebuffer.clear();
    scene.clear();
}

void AppRenderer::create(
        GLenum format,
        int width,
        int height,
        int numMultiSamples,
        int swapChainLength,
        GLuint *colorTextures) {
    EglInitExtensions();
    framebuffer.create(format, width, height, numMultiSamples, swapChainLength, colorTextures);
    if (glExtensions.EXT_sRGB_write_control) {
        // This app was originally written with the presumption that
        // its swapchains and compositor front buffer were RGB.
        // In order to have the colors the same now that its compositing
        // to an sRGB front buffer, we have to write to an sRGB swapchain
        // but with the linear->sRGB conversion disabled on write.
        GL(glDisable(GL_FRAMEBUFFER_SRGB_EXT));
    }
}

void AppRenderer::destroy() {
    framebuffer.destroy();
}

void AppRenderer::renderFrame(AppRenderer::FrameIn frameIn, Engine &engine) {
    // Update the scene matrices.
    GL(glBindBuffer(GL_UNIFORM_BUFFER, scene.sceneMatrices));
    GL(Matrix4f *sceneMatrices = (Matrix4f *) glMapBufferRange(
            GL_UNIFORM_BUFFER,
            0,
            4 * sizeof(Matrix4f) /* 2 view + 2 proj matrices */,
               GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

    if (sceneMatrices != nullptr) {
        memcpy((char *) sceneMatrices, &frameIn.view, 4 * sizeof(Matrix4f));
    }

    GL(glUnmapBuffer(GL_UNIFORM_BUFFER));
    GL(glBindBuffer(GL_UNIFORM_BUFFER, 0));

    scene.clearColor[0] = scene.clearColor[1] = scene.clearColor[2] = 0.0f;
    scene.clearColor[3] = 0.0f;

    // Render the eye images.
    framebuffer.bind(frameIn.swapChainIndex);

    GL(glEnable(GL_SCISSOR_TEST));
    GL(glDepthMask(GL_TRUE));
    GL(glEnable(GL_DEPTH_TEST));
    GL(glDepthFunc(GL_LEQUAL));
    GL(glEnable(GL_CULL_FACE));
    GL(glCullFace(GL_BACK));
    GL(glDisable(GL_BLEND));
    GL(glViewport(0, 0, framebuffer.width, framebuffer.height));
    GL(glScissor(0, 0, framebuffer.width, framebuffer.height));
    GL(glClearColor(
            scene.clearColor[0], scene.clearColor[1], scene.clearColor[2], scene.clearColor[3]));
    GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    GL(glLineWidth(3.0));


    /*if (frameIn.hasStage) {
        // stage axes
        GL(glUseProgram(scene.program.program));
        GL(glBindBufferBase(
                GL_UNIFORM_BUFFER,
                scene.program.uniformBinding[Uniform::Index::SCENE_MATRICES],
                scene.sceneMatrices));
        if (scene.program.uniformLocation[Uniform::Index::VIEW_ID] >=
            0) // NOTE: will not be present when multiview path is enabled.
        {
            GL(glUniform1i(scene.program.uniformLocation[Uniform::Index::VIEW_ID], 0));
        }
        if (scene.program.uniformLocation[Uniform::Index::MODEL_MATRIX] >= 0) {
            const Matrix4f scale = Matrix4f::Scaling(frameIn.stageScale[0], frameIn.stageScale[1],
                                                     frameIn.stageScale[2]);
            const Matrix4f stagePoseMat = Matrix4f(frameIn.stagePose);
            const Matrix4f m1 = stagePoseMat * scale;
            GL(glUniformMatrix4fv(
                    scene.program.uniformLocation[Uniform::Index::MODEL_MATRIX],
                    1,
                    GL_TRUE,
                    &m1.M[0][0]));
        }
        GL(glBindVertexArray(scene.geometries[1].vertexArrayObject));
        GL(glDrawElements(GL_LINES, scene.geometries[1].indexCount, GL_UNSIGNED_SHORT, nullptr));
        GL(glBindVertexArray(0));
        GL(glUseProgram(0));
    }*/

    //RENDER USER
    {
        auto &prg = scene.program;
        GL(glDepthMask(GL_TRUE));
        GL(glEnable(GL_DEPTH_TEST));
        GL(glDepthFunc(GL_LEQUAL));
        GL(glDisable(GL_CULL_FACE));
        GL(glEnable(GL_BLEND));
        GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        GL(glBindBufferBase(
                GL_UNIFORM_BUFFER,
                prg.uniformBinding[Uniform::Index::SCENE_MATRICES],
                scene.sceneMatrices));
        if (prg.uniformLocation[Uniform::Index::VIEW_ID] >=
            0) { // NOTE: will not be present when multiview path is enabled.
            GL(glUniform1i(prg.uniformLocation[Uniform::Index::VIEW_ID], 0));
        }

        //Piarno will render all objects
        engine.render();
    }

    {
        // Controllers TODO: migrate this to engine/Piarno
        auto &prg = scene.program;
        auto &geo = scene.geometries[1];
        GL(glUseProgram(prg.program));
        GL(glBindVertexArray(geo.vertexArrayObject));
        GL(glBindBufferBase(
                GL_UNIFORM_BUFFER,
                prg.uniformBinding[Uniform::Index::SCENE_MATRICES],
                scene.sceneMatrices));
        if (prg.uniformLocation[Uniform::Index::VIEW_ID] >=
            0) // NOTE: will not be present when multiview path is enabled.
        {
            GL(glUniform1i(prg.uniformLocation[Uniform::Index::VIEW_ID], 0));
        }
        GL(glDepthMask(GL_TRUE));
        GL(glEnable(GL_DEPTH_TEST));
        GL(glDepthFunc(GL_LEQUAL));
        GL(glDisable(GL_CULL_FACE));
        GL(glEnable(GL_BLEND));
        GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        for (int i = 0; i < 4; i++) {
            if (scene.trackedController[i].active == false) {
                continue;
            }
            Matrix4f pose(scene.trackedController[i].pose);
            Matrix4f scale;
            if (i & 1) {
                scale = Matrix4f::Scaling(0.03f, 0.03f, 0.03f);
            } else {
                scale = Matrix4f::Scaling(0.02f, 0.02f, 0.06f);
            }
            Matrix4f model = pose * scale;
            glUniformMatrix4fv(
                    prg.uniformLocation[Uniform::Index::MODEL_MATRIX], 1, GL_TRUE, &model.M[0][0]);
            GL(glDrawElements(GL_TRIANGLES, geo.indexCount, GL_UNSIGNED_SHORT, NULL));
        }
        GL(glBindVertexArray(0));
        GL(glUseProgram(0));
    }

    framebuffer.resolve();
    framebuffer.unbind();
}
