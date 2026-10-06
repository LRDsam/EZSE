#include "EZSE/Renderer/Renderer.h"

#include "EZSE/Core/Assert.h"
#include "EZSE/Core/Log.h"

#include <glad/gl.h>

namespace ezse {

namespace {

const char* vertShaderSource = R"(#version 460 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_color;

out vec3 color;

void main() {
    gl_Position = vec4(a_position, 1.0);
    color = a_color;
}
)";

const char* fragShaderSource = R"(#version 460 core

in  vec3 color;
out vec4 fragColor;

void main() {
    fragColor = vec4(color, 1.0);
}
)";



const char* GetString(GLenum name) {
    return reinterpret_cast<const char*>(glGetString(name));
}

void GLAD_API_PTR OnDebugMessage(GLenum, GLenum, GLuint, GLenum severity, GLsizei,
                                 const GLchar* message, const void*) {
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:   LogError("OpenGL: {}", message); break;
        case GL_DEBUG_SEVERITY_MEDIUM: LogWarn("OpenGL: {}", message); break;
        case GL_DEBUG_SEVERITY_LOW:    LogInfo("OpenGL: {}", message); break;
        default:                       LogTrace("OpenGL: {}", message); break;
    }
}

GLuint CompileShader(GLenum type, const char* source) {
    GLuint m_shader = glCreateShader(type);
    glShaderSource(m_shader, 1, &source, nullptr);
    glCompileShader(m_shader);

    GLint status = 0;
    glGetShaderiv(m_shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char error[512];
        glGetShaderInfoLog(m_shader, sizeof(error), nullptr, error);
        LogError("OpenGL: Error while compiling {}: {}", type, error);
        glDeleteShader(m_shader);
        return 0;
    }
    return m_shader;
}

} // namespace

std::unique_ptr<Renderer> Renderer::Create(GLLoadFunction loadFunction) {
    EZSE_ASSERT(loadFunction != nullptr, "Renderer needs a function to load OpenGL with");

    const int version = gladLoadGL(loadFunction);
    if (version == 0) {
        LogError("Failed to load OpenGL functions");
        return nullptr;
    }

#if defined(EZSE_DEBUG)
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OnDebugMessage, nullptr);
#endif

    LogInfo("Loaded OpenGL {}.{}", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
    LogInfo("Graphics card: {}", GetString(GL_RENDERER));
    LogInfo("Driver: {}", GetString(GL_VERSION));

    std::unique_ptr<Renderer> renderer(new Renderer());
    const float triangle[] = {
        0.0f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,
       -0.5f, -0.5f,  0.0f,  0.0f,  1.0f,  0.0f,
        0.5f, -0.5f,  0.0f,  0.0f,  0.0f,  1.0f
    };

    glCreateBuffers(1, &renderer->m_vertexBuffer);
    glNamedBufferStorage(renderer->m_vertexBuffer, sizeof(triangle), triangle,0);
    LogInfo("OpenGL: Vertex buffer {} created", renderer->m_vertexBuffer);

    glCreateVertexArrays(1, &renderer->m_vao);
    glVertexArrayVertexBuffer(renderer->m_vao, 0, renderer->m_vertexBuffer, 0, 6 * sizeof(float));
    glVertexArrayAttribFormat(renderer->m_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribFormat(renderer->m_vao, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
    glEnableVertexArrayAttrib(renderer->m_vao, 0);
    glEnableVertexArrayAttrib(renderer->m_vao, 1);
    glVertexArrayAttribBinding(renderer->m_vao, 0, 0);
    glVertexArrayAttribBinding(renderer->m_vao, 1, 0);
    LogInfo("OpenGL: Vertex Array {} created", renderer->m_vao);

    auto vertShader = CompileShader(GL_VERTEX_SHADER,vertShaderSource);
    if (vertShader == 0) {
        return nullptr;
    }
    auto fragShader = CompileShader(GL_FRAGMENT_SHADER, fragShaderSource);
    if (fragShader == 0) {
        glDeleteShader(vertShader);
        return nullptr;
    }
    renderer->m_shader = glCreateProgram();
    glAttachShader(renderer->m_shader, vertShader);
    glAttachShader(renderer->m_shader, fragShader);
    glLinkProgram(renderer->m_shader);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    GLint status = 0;
    glGetProgramiv(renderer->m_shader, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        char error[512];
        glGetProgramInfoLog(renderer->m_shader, sizeof(error), nullptr, error);
        LogError("OpenGL: Error while linking shader program: {}", error);
        return nullptr;
    }



    return renderer;
}

void Renderer::DrawTriangle() {
    glUseProgram(m_shader);
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}


void Renderer::Clear(float red, float green, float blue) {
    glClearColor(red, green, blue, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

Renderer::~Renderer() {
    glDeleteBuffers(1, &m_vertexBuffer);
    glDeleteVertexArrays(1, &m_vao);
    glDeleteProgram(m_shader);
}

} // namespace ezse
