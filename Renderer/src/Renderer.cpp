#include "EZSE/Renderer/Renderer.h"

#include "EZSE/Core/Assert.h"
#include "EZSE/Core/Log.h"

#include <glad/gl.h>

#include <algorithm>
#include <bit>
#include <cstddef>

namespace ezse {

namespace {

constexpr GLuint c_cameraBindingPoint = 0;
constexpr GLuint c_lightBindingPoint = 1;
constexpr GLuint c_textureUnit = 0;

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

// The Light block in the shaders, laid out as std140: a vec3 takes the room of a vec4 (16 bytes),
// so each vec3 is followed by one float of padding.
struct LightData {
    float direction[3];
    float padding0 = 0.0f;
    float color[3];
    float padding1 = 0.0f;
    float ambient[3];
    float padding2 = 0.0f;
};
static_assert(sizeof(LightData) == 48);

// The Camera block in the shaders, laid out as std140: the mat4 takes 64 bytes, and the vec3 after
// it takes the room of a vec4, so it is followed by one float of padding.
struct CameraData {
    Mat4 viewProjection;
    float position[3];
    float padding0 = 0.0f;
};
static_assert(sizeof(CameraData) == 80);

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
    glCreateBuffers(1, &renderer->m_cameraBuffer);
    glNamedBufferStorage(renderer->m_cameraBuffer, sizeof(CameraData), nullptr, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, c_cameraBindingPoint, renderer->m_cameraBuffer);

    glCreateBuffers(1, &renderer->m_lightBuffer);
    glNamedBufferStorage(renderer->m_lightBuffer, sizeof(LightData), nullptr, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, c_lightBindingPoint, renderer->m_lightBuffer);

    const std::uint8_t whitePixel[] = { 255, 255, 255, 255 };
    renderer->m_whiteTexture = renderer->CreateTexture(1, 1, whitePixel);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    return renderer;
}

void Renderer::SetViewport(int width, int height) {
    if (width > 0 && height > 0) {
        glViewport(0, 0, width, height);
    }
}

MeshHandle Renderer::CreateMesh(std::span<const Vertex> vertices, std::span<const std::uint32_t> indices) {
    EZSE_ASSERT(!vertices.empty(), "CreateMesh needs at least one vertex");
    EZSE_ASSERT(!indices.empty(), "CreateMesh needs at least one index");
    EZSE_ASSERT(indices.size() % 3 == 0, "CreateMesh needs indices to be a multiple of 3");
    for(const std::uint32_t index : indices) {
        EZSE_ASSERT(index < vertices.size(), "CreateMesh got an index that points past the last vertex");
    }

    MeshData meshData;
    glCreateBuffers(1, &meshData.vertexBufferObj);
    glNamedBufferStorage(meshData.vertexBufferObj, vertices.size_bytes(), vertices.data(), 0);
    LogInfo("OpenGL: Vertex buffer {} created", meshData.vertexBufferObj);

    glCreateVertexArrays(1, &meshData.vertexArrayObj);
    glVertexArrayVertexBuffer(meshData.vertexArrayObj, 0, meshData.vertexBufferObj, 0,sizeof(Vertex));
    glVertexArrayAttribFormat(meshData.vertexArrayObj, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribFormat(meshData.vertexArrayObj, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
    glVertexArrayAttribFormat(meshData.vertexArrayObj, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
    glEnableVertexArrayAttrib(meshData.vertexArrayObj, 0);
    glEnableVertexArrayAttrib(meshData.vertexArrayObj, 1);
    glEnableVertexArrayAttrib(meshData.vertexArrayObj, 2);
    glVertexArrayAttribBinding(meshData.vertexArrayObj, 0, 0);
    glVertexArrayAttribBinding(meshData.vertexArrayObj, 1, 0);
    glVertexArrayAttribBinding(meshData.vertexArrayObj, 2, 0);
    LogInfo("OpenGL: Vertex Array {} created", meshData.vertexArrayObj);

    glCreateBuffers(1, &meshData.indexBufferObj);
    glNamedBufferStorage(meshData.indexBufferObj, indices.size_bytes(), indices.data(), 0);
    glVertexArrayElementBuffer(meshData.vertexArrayObj, meshData.indexBufferObj);
    LogInfo("OpenGL: Index buffer {} created", meshData.indexBufferObj);

    meshData.indexCount = static_cast<unsigned int>(indices.size());
    m_meshData.push_back(meshData);
    return MeshHandle{ static_cast<std::uint32_t>(m_meshData.size()) };
}

void Renderer::DestroyMesh(MeshHandle mesh) {
    MeshData& meshData = GetMeshData(mesh);

    glDeleteBuffers(1, &meshData.vertexBufferObj);
    glDeleteBuffers(1, &meshData.indexBufferObj);
    glDeleteVertexArrays(1, &meshData.vertexArrayObj);

    meshData = MeshData{};
}

void Renderer::DrawMesh(MeshHandle mesh, ShaderHandle shader, TextureHandle texture, const Mat4& model) {
    const MeshData& meshData = GetMeshData(mesh);
    const ShaderData& shaderData = GetShaderData(shader);

    glUseProgram(shaderData.shaderProgram);
    glBindVertexArray(meshData.vertexArrayObj);
    // Id 0 draws with the white texture, so a shader always has a texture to read.
    const TextureHandle drawTexture = texture.id == 0 ? m_whiteTexture : texture;
    glBindTextureUnit(c_textureUnit, GetTextureData(drawTexture).textureObj);
    glProgramUniformMatrix4fv(
        shaderData.shaderProgram,
        shaderData.modelLocation,
        1,
        GL_FALSE,
        &model.m[0][0]);
    glDrawElements(GL_TRIANGLES, meshData.indexCount, GL_UNSIGNED_INT, nullptr);
}

TextureHandle Renderer::CreateTexture(int width, int height, std::span<const std::uint8_t> pixels) {
    EZSE_ASSERT(width > 0 && height > 0, "CreateTexture needs a width and height larger than 0");
    EZSE_ASSERT(pixels.size() == static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4,
        "CreateTexture needs exactly width * height * 4 bytes (RGBA)");

    // One level for every halving of the largest side, down to 1 pixel: 256 gives 9 levels.
    const int levels = static_cast<int>(std::bit_width(static_cast<unsigned int>(std::max(width, height))));

    TextureData textureData;
    glCreateTextures(GL_TEXTURE_2D, 1, &textureData.textureObj);
    glTextureStorage2D(textureData.textureObj, levels, GL_RGBA8, width, height);
    glTextureSubImage2D(textureData.textureObj, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glGenerateTextureMipmap(textureData.textureObj);

    glTextureParameteri(textureData.textureObj, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(textureData.textureObj, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(textureData.textureObj, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(textureData.textureObj, GL_TEXTURE_WRAP_T, GL_REPEAT);
    LogInfo("OpenGL: Texture {} created ({}x{}, {} levels)", textureData.textureObj, width, height, levels);

    m_textureData.push_back(textureData);
    return TextureHandle{ static_cast<std::uint32_t>(m_textureData.size()) };
}

void Renderer::DestroyTexture(TextureHandle texture) {
    TextureData& textureData = GetTextureData(texture);
    glDeleteTextures(1, &textureData.textureObj);
    textureData = TextureData{};
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

    shaderData.modelLocation = glGetUniformLocation(shaderData.shaderProgram, "u_model");
    if (shaderData.modelLocation < 0) {
        LogWarn("Uniform u_model not found in shader!");
    }

    m_shaderData.push_back(shaderData);
    return ShaderHandle{ static_cast<std::uint32_t>(m_shaderData.size()) };

}

void Renderer::DestroyShader(ShaderHandle shader) {
    ShaderData& shaderData = GetShaderData(shader);
    glDeleteProgram(shaderData.shaderProgram);
    shaderData = ShaderData{};

}

void Renderer::SetCamera(const Mat4& view, const Mat4& projection, const Vec3& position) {
    const CameraData cameraData = {
        .viewProjection = projection * view,
        .position{position.x, position.y, position.z}};
    glNamedBufferSubData(m_cameraBuffer, 0, sizeof(CameraData), &cameraData);
}

void Renderer::SetDirectionalLight(const Vec3& direction, const Vec3& color) {
    const Vec3 lightDirection = Normalize(direction);
    const LightData lightData = {
        .direction{lightDirection.x, lightDirection.y, lightDirection.z},
        .color{color.x, color.y, color.z}};
    glNamedBufferSubData(
        m_lightBuffer,
        0,
        offsetof(LightData, ambient),
        &lightData);
}

void Renderer::SetAmbientLight(const Vec3& color) {
    const float ambient[3] = {color.x, color.y, color.z};
    glNamedBufferSubData(
        m_lightBuffer,
        offsetof(LightData, ambient),
        sizeof(ambient),
        ambient);
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

Renderer::TextureData& Renderer::GetTextureData(TextureHandle texture) {
    EZSE_ASSERT(texture.id > 0 && texture.id <= m_textureData.size(), "Texture handle is out of range");
    TextureData& textureData = m_textureData[texture.id - 1];
    EZSE_ASSERT(textureData.textureObj != 0, "Texture handle refers to a destroyed texture");
    return textureData;
}

void Renderer::Clear(float red, float green, float blue) {
    glClearColor(red, green, blue, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

Renderer::~Renderer() {
    for (auto& meshData : m_meshData) {
        glDeleteBuffers(1, &meshData.vertexBufferObj);
        glDeleteBuffers(1, &meshData.indexBufferObj);
        glDeleteVertexArrays(1, &meshData.vertexArrayObj);
    }

    for (auto& shaderData : m_shaderData) {
        glDeleteProgram(shaderData.shaderProgram);
    }

    for (auto& textureData : m_textureData) {
        glDeleteTextures(1, &textureData.textureObj);
    }

    glDeleteBuffers(1, &m_cameraBuffer);
    glDeleteBuffers(1, &m_lightBuffer);

}

} // namespace ezse
