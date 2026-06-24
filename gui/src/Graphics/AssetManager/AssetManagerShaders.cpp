/*
** EPITECH PROJECT, 2026
** zappy_gui
** File description:
** AssetManager.cpp
*/

#include "BoundingBox.hpp"
#include "Graphics/AssetManager.hpp"
#include "Graphics/GraphicsErrors.hpp"
#include "Logging/Logger.hpp"
#include "Model.hpp"
#include "raylib.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <memory>

namespace zappy {

void AssetManager::_loadShaders() {
    // explanations of... stuff:
    // Pipeline: The sequence of steps the GPU takes to convert 3D data into 2D pixels

    // VS (Vertex Shader): The first programmable stage of the pipeline. It executes once per
    // vertex. Its primary role is transforming 3D coordinates into 2D screen space (gl_Position).

    // FS (Fragment Shader): The final programmable stage. It executes once per rasterized pixel
    // (fragment). Its primary role is calculating the final RGBA color output (finalColor).

    // R"(...)": C++ Raw String Literal. Allows embedding multi-line GLSL code without escaping.
    // in / out: Data flow qualifiers. 'in' receives data from the previous pipeline stage (e.g. CPU

    // -> VS, or VS -> FS), and 'out' passes data to the next stage.

    // uniform: Global variables set by the CPU that remain constant for the entire draw call.

    // vec2/3/4: Mathematical vectors representing points, directions, or RGBA colors.
    // mat4: A 4x4 transformation matrix (used to translate, rotate, and scale geometry).

    // Hardware Instancing Shader
    // This shader handles rendering multiple instances of the same model in a single draw call.
    // The vertex shader (instancingVS) applies an instance-specific transformation matrix
    // (instanceTransform) to position each instance correctly in world space, bypassing the
    // CPU overhead of individual draw commands.
    const char* instancingVS = R"(#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in mat4 instanceTransform;

uniform mat4 mvp;

out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

void main()
{
    fragTexCoord = vertexTexCoord;
    fragColor = vec4(1.0);
    gl_Position = mvp * instanceTransform * vec4(vertexPosition, 1.0);
}
)";

    // The fragment shader (instancingFS) applies texture and color tinting per pixel
    // It also discards fragments with an alpha value < 0.1 to correctly render
    // textures with transparency cutouts (like leaves or decals) without writing
    // to the depth buffer and occluding geometry behind them
    const char* instancingFS = R"(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

out vec4 finalColor;

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);
    if (texelColor.a < 0.1) discard; // Support alpha cutouts
    finalColor = texelColor * colDiffuse * fragColor;
}
)";

    _shaders["instancing"] =
        std::make_unique<raylib::Shader>(::LoadShaderFromMemory(instancingVS, instancingFS));

    // Alpha Cutout Shader
    // Used primarily for flat 2D terrain decals. It ensures fragments with low alpha
    // do not write to the depth buffer, preventing transparent boundary areas from
    // incorrectly hiding the underlying base terrain tiles
    const char* alphaFS = R"(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
out vec4 finalColor;
void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);
    if (texelColor.a < 0.1) discard;
    finalColor = texelColor * fragColor;
}
)";

    _shaders["alphaCutout"] =
        std::make_unique<raylib::Shader>(::LoadShaderFromMemory(nullptr, alphaFS));

    const char* incantationFS = R"(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;

uniform float time;

out vec4 finalColor;

void main()
{
    vec2 p = fragTexCoord - 0.5;
    float r = length(p);
    float a = atan(p.y, p.x);
    // Smoothness factor for anti-aliasing
    float sm = 0.01;
    // Outer boundary (don't draw outside the quad)
    if (r > 0.5) discard;
    // Rotation
    float a1 = a + time * 0.5; // Outer rotates one way
    float a2 = a - time * 0.3; // Inner rotates another
    float intensity = 0.0;
    // Outer thick ring
    intensity += smoothstep(0.02, 0.0, abs(r - 0.45));
    // Outer dashed/runic ring
    float dashRing = smoothstep(0.03, 0.0, abs(r - 0.4));
    float dashes = step(0.0, sin(a1 * 30.0));
    intensity += dashRing * dashes;

    // Inner thin ring
    intensity += smoothstep(0.01, 0.0, abs(r - 0.32));

    // Hexagram (Two overlapping triangles)
    // Distance to a triangle from center
    float tri1 = max(max(r * cos(a2), r * cos(a2 + 2.09439)), r * cos(a2 - 2.09439));
    float tri2 = max(max(r * cos(a2 + 3.14159), r * cos(a2 + 3.14159 + 2.09439)), r * cos(a2 + 3.14159 - 2.09439));
    intensity += smoothstep(0.01, 0.0, abs(tri1 - 0.15));
    intensity += smoothstep(0.01, 0.0, abs(tri2 - 0.15));
    // Core pulsing dot
    float pulse = sin(time * 4.0) * 0.5 + 0.5;
    intensity += smoothstep(0.05 + pulse * 0.02, 0.0, r);

    // Magic fire colors
    vec3 primaryColor = vec3(1.0, 1.0, 1.0); // Bright white
    vec3 secondaryColor = vec3(0.8, 0.9, 1.0); // Light heavenly blue
    vec3 tertiaryColor = vec3(0.5, 0.7, 1.0); // Deep heavenly blue
    float coreGlow = smoothstep(0.4, 0.0, r) * 0.5 * pulse;
    vec3 finalRGB = mix(tertiaryColor, primaryColor, intensity) * (intensity + coreGlow);
    float finalAlpha = (intensity + coreGlow) * fragColor.a;

    finalColor = vec4(finalRGB, finalAlpha);
    if (finalColor.a < 0.05) discard;
}
)";

    _shaders["incantation_aura"] =
        std::make_unique<raylib::Shader>(::LoadShaderFromMemory(nullptr, incantationFS));

    const char* wormholeFS = R"(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;

uniform float time;

out vec4 finalColor;

void main()
{
    vec2 p = fragTexCoord - 0.5;
    float r = length(p);
    
    if (r > 0.5) discard;
    
    float a = atan(p.y, p.x);
    
    float spiral = sin(a * 5.0 + r * 20.0 - time * 5.0);
    float spiral2 = sin(a * 3.0 + r * 15.0 - time * 3.0);
    
    float wobble = sin(r * 30.0 - time * 8.0) * 0.5 + 0.5;
    float intensity = (spiral * 0.5 + 0.5) * (spiral2 * 0.5 + 0.5) + wobble * 0.5;
    
    vec3 colDark = vec3(0.2, 0.0, 0.4);   // Dark purple
    vec3 colMid = vec3(0.6, 0.1, 0.8);    // Neon purple
    vec3 colLight = vec3(0.0, 1.0, 0.8);  // Cyan/green
    
    float edge = smoothstep(0.5, 0.4, r);
    float core = smoothstep(0.2, 0.0, r);
    
    vec3 finalRGB = mix(colDark, colMid, intensity);
    finalRGB = mix(finalRGB, colLight, core + intensity * 0.3 * (1.0 - edge));
    
    float finalAlpha = edge * (0.5 + intensity * 0.5) * fragColor.a;
    
    finalColor = vec4(finalRGB, finalAlpha);
    if (finalColor.a < 0.05) discard;
}
)";
    _shaders["wormhole_portal"] =
        std::make_unique<raylib::Shader>(::LoadShaderFromMemory(nullptr, wormholeFS));

    auto& shader = *_shaders["instancing"];
    for (auto& [name, model] : _models) {
        if (name == "robot") {
            continue;
        }
        for (int i = 0; i < model->materialCount; i++) {
            model->materials[i].shader = shader;
        }
    }
}

} // namespace zappy
