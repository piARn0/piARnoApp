#pragma once

#include <GLES3/gl3.h>
#include "OVR_Math.h"
#include <vector>
#include <openxr/openxr.h>

#ifndef NUM_EYES
#define NUM_EYES 2
#endif

// Represents a GL shader program
struct Program {
    static constexpr int MAX_PROGRAM_UNIFORMS = 8;
    static constexpr int MAX_PROGRAM_TEXTURES = 8;

    void clear();

    bool create(const char *vertexSource, const char *fragmentSource);

    void destroy();

    GLuint program;
    GLuint vertexShader;
    GLuint fragmentShader;
    // These will be -1 if not used by the program.
    GLint uniformLocation[MAX_PROGRAM_UNIFORMS]; // ProgramUniforms[].name
    GLint uniformBinding[MAX_PROGRAM_UNIFORMS]; // ProgramUniforms[].name
    GLint textures[MAX_PROGRAM_TEXTURES]; // Texture%i
    GLint colorLocation; //fragmentColor
};


using vertex_t = float;
using color_t = uint8_t;
using index_t = uint16_t;

// Represents a mesh loaded into the GPU
struct Geometry {
    /// Interface

    // creates an UNINITIALIZED(!) object, you MUST manually call create() and updateX before use!
    Geometry();

    // create a new GL object with 3D vertexPositions, (per-vertex) RGBA colors, and indices
    Geometry(const std::vector<vertex_t> &vertexPositions,
             const std::vector<color_t> &colors,
             const std::vector<index_t> &indices,
             GLenum draw_mode = GL_TRIANGLES);

    // create a new GL object with 3D vertexPositions and indices
    Geometry(const std::vector<vertex_t> &vertexPositions,
             const std::vector<index_t> &indices,
             GLenum draw_mode = GL_TRIANGLES);

    void create(bool global_color, GLenum draw_mode = GL_TRIANGLES);

    void clear();

    void destroy();

    void createVAO();

    void destroyVAO();

    void updateVertices(const std::vector<vertex_t> &vertexPositions);
    void updateColors(const std::vector<color_t> &colors);
    void updateIndices(const std::vector<index_t> &indices);

    void render(const OVR::Matrix4f &transform);

    /// Internal
    GLuint vertexBuffer;
    GLuint colorBuffer;
    GLuint indexBuffer;
    size_t vertexCount;
    size_t indexCount;

    GLuint vertexArrayObject;

    Program *program = nullptr;
    GLenum draw_mode;
    bool global_color;
};

struct Framebuffer {
    void clear();

    bool create(
            const GLenum colorFormat,
            const int width,
            const int height,
            const int multisamples,
            const int swapChainLength,
            GLuint *colorTextures);

    void destroy();

    void bind(int element);

    void unbind();

    void resolve();

    int width;
    int height;
    int multisamples;
    int swapChainLength;
    struct Element {
        GLuint colorTexture;
        GLuint depthTexture;
        GLuint frameBufferObject;
    };
    Element *elements;
};

struct TrackedController {
    void clear();

    bool active;
    OVR::Posef pose;
};

struct Scene {
    void clear();

    void create();

    void destroy();

    bool isCreated();

    void setClearColor(const float *c);

    void createVAOs();

    void destroyVAOs();

    bool createdScene;
    bool createdVAOs;
    GLuint sceneMatrices;

    Program program, program_uniform_color;
    std::vector<Geometry> geometries;

    float clearColor[4];
    TrackedController trackedController[4]; // left aim, left grip, right aim, right grip

    // States for all defined pressing actions
    XrBool32 leftTriggerPressed;
    XrBool32 rightTriggerPressed;

    XrBool32 leftSqueezePressed;
    XrBool32 rightSqueezePressed;

    XrBool32 xButtonPressed;
    XrBool32 yButtonPressed;

    XrBool32 aButtonPressed;
    XrBool32 bButtonPressed;

    // States for all defined holding actions
    float rightTriggerHoldLevel;
};

class Engine;

struct AppRenderer {
    void clear();

    void create(
            GLenum format,
            int width,
            int height,
            int numMultiSamples,
            int swapChainLength,
            GLuint *colorTextures);

    void destroy();

    struct FrameIn {
        int swapChainIndex;
        OVR::Matrix4f view[NUM_EYES];
        OVR::Matrix4f proj[NUM_EYES];
        bool hasStage;
        OVR::Posef stagePose;
        OVR::Vector3f stageScale;
    };

    void renderFrame(FrameIn frameIn, Engine &engine);

    Framebuffer framebuffer;
    Scene scene;
};
