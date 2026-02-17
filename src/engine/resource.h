#ifndef GAME_RESOURCE_H
#define GAME_RESOURCE_H

#include <string>
#include <optional>
#include <unordered_map>
#include <memory>
#include <ranges>
#include <utility>
#include <stdexcept>

#include "Files.h"
#include "logging.h"
#include "raylib.h"
#include "vec.h"

class Resource {
protected:

    static Logging::Logger& getLogger() {
        static auto logger = NEW_LOGGER(Resource);
        return logger;
    }

    std::string path;

public:
    explicit Resource(const std::string& path) {
        this->path = Files::path(path);
    }

    [[nodiscard]] virtual bool isLoaded() = 0;

    virtual void load() {
        if (isLoaded()) return;
        doLoad();
    }

    virtual void unload() {
        if (!isLoaded()) return;
        doUnload();
    }

    virtual ~Resource() = default;

private:
    virtual void doLoad() = 0;
    virtual void doUnload() = 0;
};

class TextureResource : public Resource {
    std::optional<Texture2D> texture = std::nullopt;
    std::optional<RenderTexture2D> renderTexture = std::nullopt;

public:
    explicit TextureResource(const std::string &path)
        : Resource(path) {
    }

    [[nodiscard]] static TextureResource* generate(const std::function<RenderTexture2D()>& f) {
        const auto rtex = new TextureResource("<generated>");

        rtex->renderTexture = f();
        rtex->texture = rtex->renderTexture->texture;

        return rtex;
    }

    bool isLoaded() override {
        return texture.has_value() && IsTextureValid(*texture);
    }

    void doLoad() override {
        getLogger().log("Loading texture. path=%s", path.c_str());
        texture = LoadTexture(path.c_str());
        if (!IsTextureValid(*texture)) {
            getLogger().logWarn("Texture not found! path=%s", path.c_str());
            texture = std::nullopt;
        }
    }

    void doUnload() override {
        if (renderTexture.has_value()) {
            UnloadRenderTexture(*renderTexture);
        } else {
            UnloadTexture(*texture);
        }
    }

    [[nodiscard]] bool isRenderTexture() const {
        return renderTexture.has_value();
    }

    void updateTextureFromRenderTexture() {
        if (!renderTexture.has_value()) return;

        texture = renderTexture->texture;
    }

    [[nodiscard]] std::optional<Texture2D> getTexture() const {
        return texture;
    }

    [[nodiscard]] Vec2 getDimensions() const {
        if (texture.has_value()) {
            return {static_cast<float>(texture->width), static_cast<float>(texture->height)};
        }

        return {0, 0};
    }

    void render(const Vec2 pos, const float rotation, const float scale, const Color& tint) {
        renderEx(pos, Vec2::zero(), rotation, scale, tint);
    }

    void renderEx(const Vec2 pos, const Vec2 offset, const float rotation, const float scale, const Color& tint) {
        // TODO, draw default 'missing texture' texture
        if (!isLoaded()) throw std::runtime_error("attempt to render unloaded resource");

        const auto w = static_cast<float>(texture->width);
        const auto h = static_cast<float>(texture->height);

        const auto sw = w * scale;
        const auto sh = h * scale;

        DrawTexturePro(*texture,
            {offset.x, offset.y, w, h},
            {pos.x, pos.y, sw, sh},
            {sw*0.5f, sh*0.5f},
            rotation * RAD2DEG,
            tint);
    }
};

class FragmentShader : public Resource {
    std::optional<Shader> shader = std::nullopt;

    static Logging::Logger& getLogger() {
        static Logging::Logger logger = NEW_LOGGER(FragmentShader);
        return logger;
    }
public:
    explicit FragmentShader(const std::string &path)
        : Resource(path) {
    }

    bool isLoaded() override {
        return shader.has_value() && IsShaderValid(*shader);
    }

    void doLoad() override {
        getLogger().log("Loading fragment shader. path=%s", path.c_str());
        shader = LoadShader(nullptr, path.c_str());
        if (!IsShaderValid(*shader)) {
            getLogger().logWarn("Shader not found! path=%s", path.c_str());
            shader = std::nullopt;
        }
    }

    void doUnload() override {
        UnloadShader(*shader);
    }

    void setField(const std::string_view& fieldName, const float value) const {
        if (!shader.has_value()) {
            getLogger().logWarn("Shader not found, but field was set");
            return;
        }

        const int index = GetShaderLocation(*shader, fieldName.data());

        SetShaderValue(*shader, index, &value, SHADER_UNIFORM_FLOAT);
    }

    void setField(const std::string_view& fieldName, const Color value) const {
        if (!shader.has_value()) {
            getLogger().logWarn("Shader not found, but field was set");
            return;
        }

        const int index = GetShaderLocation(*shader, fieldName.data());

        SetShaderValue(*shader, index, &value, SHADER_UNIFORM_VEC4);
    }

    void setField(const std::string_view& fieldName, const Vec2 value) const {
        if (!shader.has_value()) {
            getLogger().logWarn("Shader not found, but field was set");
            return;
        }

        const int index = GetShaderLocation(*shader, fieldName.data());

        const Vector2 r = value;

        SetShaderValue(*shader, index, &r, SHADER_UNIFORM_VEC2);
    }

    void begin() const {
        if (!shader.has_value()) {
            getLogger().logWarn("Attempted to use nullopt shader!");
            return;
        }
        BeginShaderMode(*shader);
    }

    void end() const {
        EndShaderMode();
    }

    void with(const std::function<void()>& f) const {
        begin();
        f();
        end();
    }
};

class ResourceManager {
    std::unordered_map<std::string, Resource*> resources;

    static Logging::Logger& getLogger() {
        static auto logger = NEW_LOGGER(ResourceManager);
        return logger;
    }

public:
    void registerResource(const std::string& name, Resource* res) {
        res->load();
        resources.emplace(name, res);
    }

    template<typename T>
        requires std::is_base_of_v<Resource, T>
    [[nodiscard]] std::optional<T*> getResource(const std::string& name) {
        if (const auto it = resources.find(name); it != resources.end()) {
            return dynamic_cast<T*>(it->second);
        }

        getLogger().logWarn("Resource not found: %s", name.c_str());

        return std::nullopt;
    }

    ~ResourceManager() {
        for (const auto val: resources | std::views::values) {
            val->unload();
            delete val;
        }
    }
};

#endif //GAME_RESOURCE_H