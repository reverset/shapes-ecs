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
#include "raylib.h"
#include "vec.h"

class Resource {
protected:
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

public:
    explicit TextureResource(const std::string &path)
        : Resource(path) {
    }

    bool isLoaded() override {
        return texture.has_value() && IsTextureValid(*texture);
    }

    void doLoad() override {
        texture = LoadTexture(path.c_str());
    }

    void doUnload() override {
        UnloadTexture(*texture);
    }

    [[nodiscard]] std::optional<Texture2D> getTexture() const {
        return texture;
    }

    void render(const Vec2 pos, const float rotation, const float scale, const Color& tint) {
        renderEx(pos, VEC2_ZERO, rotation, scale, tint);
    }

    void renderEx(const Vec2 pos, const Vec2 offset, const float rotation, const float scale, const Color& tint) {
        if (!isLoaded()) throw std::runtime_error("attempt to render unloaded resource");

        const auto w = static_cast<float>(texture->width);
        const auto h = static_cast<float>(texture->height);

        const auto sw = w * scale;
        const auto sh = h * scale;

        DrawTexturePro(*texture,
            {offset.x, offset.y, w, h},
            {pos.x, pos.y, sw, sh},
            {sw/2.0f, sh/2.0f},
            rotation * RAD2DEG,
            tint);
    }
};

class ResourceManager {
    std::unordered_map<std::string, Resource*> resources;

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