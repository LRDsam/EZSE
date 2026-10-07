#include "EZSE/Renderer/Renderer.h"

#include "EZSE/Core/Assert.h"
#include "EZSE/Core/Log.h"

#include <glad/gl.h>

#include <cstddef>


namespace ezse {

namespace {

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

GLuint CompileShader(GLenum type, std::string_view source) {
    const char* sourceText = source.data();
    const GLint sourceLength = static_cast<GLint>(source.size());

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &sourceText, &sourceLength);
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

void Renderer::DrawMesh(MeshHandle mesh, ShaderHandle shader) {
    const MeshData& meshData = GetMeshData(mesh);
    const ShaderData& shaderData = GetShaderData(shader);

    glUseProgram(shaderData.shaderProgram);
    glBindVertexArray(meshData.vertexArrayObj);
    glDrawArrays(GL_TRIANGLES, 0, meshData.vertexCount);
}

ShaderHandle Renderer::CreateShader(std::string_view vertexSource, std::string_view fragmentSource) {
    auto vertShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
    if (vertShader == 0) {
        return {};
    }
    auto fragShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (fragShader == 0) {
        glDeleteShader(vertShader);
        return {};
    }
    ShaderData shaderData;
    shaderData.shaderProgram = glCreateProgram();
    glAttachShader(shaderData.shaderProgram, vertShader);
    glAttachShader(shaderData.shaderProgram, fragShader);
    glLinkProgram(shaderData.shaderProgram);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    GLint status = 0;
    glGetProgramiv(shaderData.shaderProgram, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        char error[512];
        glGetProgramInfoLog(shaderData.shaderProgram, sizeof(error), nullptr, error);
        LogError("OpenGL: Error while linking shader program: {}", error);
        glDeleteProgram(shaderData.shaderProgram);
        return {};
    }

    m_shaderData.push_back(shaderData);
    return ShaderHandle{ static_cast<std::uint32_t>(m_shaderData.size()) };
    
}

void Renderer::DestroyShader(ShaderHandle shader) {
    ShaderData& shaderData = GetShaderData(shader);
    glDeleteProgram(shaderData.shaderProgram);
    shaderData = ShaderData{};

}

Renderer::MeshData& Renderer::GetMeshData(MeshHandle mesh) {
    EZSE_ASSERT(mesh.id > 0 && mesh.id <= m_meshData.size(), "Mesh handle is out of range");

    MeshData& meshData = m_meshData[mesh.id - 1];
    EZSE_ASSERT(meshData.vertexArrayObj != 0, "Mesh handle refers to a destroyed mesh");
    return meshData;
}

Renderer::ShaderData& Renderer::GetShaderData(ShaderHandle shader) {
    EZSE_ASSERT(shader.id > 0 && shader.id <= m_shaderData.size(), "Shader handle is out of range");

    ShaderData& shaderData = m_shaderData[shader.id - 1];
    EZSE_ASSERT(shaderData.shaderProgram != 0, "Shader handle refers to a destroyed shader");
    return shaderData;
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
    
    for (auto& shaderData : m_shaderData) {
        glDeleteProgram(shaderData.shaderProgram);
        
    }
}

} // namespace ezse
