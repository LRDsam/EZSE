#include "EZSE/Renderer/Renderer.h"

#include "EZSE/Core/Assert.h"
#include "EZSE/Core/Log.h"

#include <glad/gl.h>

#include <cstddef>


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
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char error[512];
        glGetShaderInfoLog(shader, sizeof(error), nullptr, error);
        LogError("OpenGL: Error while compiling {}: {}", type, error);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
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

MeshHandle Renderer::CreateMesh(std::span<const Vertex> vertices) {
    EZSE_ASSERT(!vertices.empty(), "CreateMesh needs at least one vertex");

    MeshData meshData;
    glCreateBuffers(1, &meshData.vertexBufferObj);
    glNamedBufferStorage(meshData.vertexBufferObj, vertices.size_bytes(), vertices.data(), 0);
    LogInfo("OpenGL: Vertex buffer {} created", meshData.vertexBufferObj);

    glCreateVertexArrays(1, &meshData.vertexArrayObj);
    glVertexArrayVertexBuffer(meshData.vertexArrayObj, 0, meshData.vertexBufferObj, 0,sizeof(Vertex));
    glVertexArrayAttribFormat(meshData.vertexArrayObj, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribFormat(meshData.vertexArrayObj, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex,color));
    glEnableVertexArrayAttrib(meshData.vertexArrayObj, 0);
    glEnableVertexArrayAttrib(meshData.vertexArrayObj, 1);
    glVertexArrayAttribBinding(meshData.vertexArrayObj, 0, 0);
    glVertexArrayAttribBinding(meshData.vertexArrayObj, 1, 0);
    LogInfo("OpenGL: Vertex Array {} created", meshData.vertexArrayObj);
    meshData.vertexCount = static_cast<unsigned int>(vertices.size());
    m_meshData.push_back(meshData);
    return MeshHandle{ static_cast<std::uint32_t>(m_meshData.size()) };
}

void Renderer::DestroyMesh(MeshHandle mesh) {
    MeshData& meshData = GetMeshData(mesh);

    glDeleteBuffers(1, &meshData.vertexBufferObj);
    glDeleteVertexArrays(1, &meshData.vertexArrayObj);
    meshData = MeshData{};
}

void Renderer::DrawMesh(MeshHandle mesh) {
    const MeshData& meshData = GetMeshData(mesh);

    glUseProgram(m_shader);
    glBindVertexArray(meshData.vertexArrayObj);
    glDrawArrays(GL_TRIANGLES, 0, meshData.vertexCount);
}

Renderer::MeshData& Renderer::GetMeshData(MeshHandle mesh) {
    EZSE_ASSERT(mesh.id > 0 && mesh.id <= m_meshData.size(), "Mesh handle is out of range");

    MeshData& meshData = m_meshData[mesh.id - 1];
    EZSE_ASSERT(meshData.vertexArrayObj != 0, "Mesh handle refers to a destroyed mesh");
    return meshData;
}

void Renderer::Clear(float red, float green, float blue) {
    glClearColor(red, green, blue, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

Renderer::~Renderer() {
    for (auto& meshData : m_meshData) {
        glDeleteBuffers(1, &meshData.vertexBufferObj);
        glDeleteVertexArrays(1, &meshData.vertexArrayObj);
    }
    
    glDeleteProgram(m_shader);
}

} // namespace ezse
