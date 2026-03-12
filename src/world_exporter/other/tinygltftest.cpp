//
// Date       : 12/03/2026
// Project    : world_exporter
// Author     : -Ry
//

#include <filesystem>
#include <string>
#include <format>
#include <iostream>
#include <fstream>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tinygltf/tiny_gltf.h"

////////////////////////////////////////////////////////////////////////////////
// | PREAMBLE |
////////////////////////////////////////////////////////////////////////////////

#define LOG_INFO(...) (std::cout << std::format(__VA_ARGS__) << "\n")

namespace {
namespace fs = std::filesystem;
void create_triangle_gltf_example();
void load_triangle_gltf();
}  // namespace

////////////////////////////////////////////////////////////////////////////////
// | ENTRY |
////////////////////////////////////////////////////////////////////////////////

int main() {
    LOG_INFO("exporting example scene and asset");
    fs::create_directories(fs::current_path() / "_export");
    create_triangle_gltf_example();
    load_triangle_gltf();
}

////////////////////////////////////////////////////////////////////////////////
// | IMPL |
////////////////////////////////////////////////////////////////////////////////

namespace {

void create_triangle_gltf_example() {
    // example from - tinygltf/examples/build-gltf/create_triangle_gltf.cpp
    // Create a model with a single mesh and save it as a gltf file
    tinygltf::Model m;
    tinygltf::Scene scene;
    tinygltf::Mesh mesh;
    tinygltf::Primitive primitive;
    tinygltf::Node node1;
    tinygltf::Node node2;
    tinygltf::Buffer buffer;
    tinygltf::BufferView bufferView1;  // indices
    tinygltf::BufferView bufferView2;  // vertices
    tinygltf::BufferView bufferView3;  // uv's
    tinygltf::Accessor accessor1;
    tinygltf::Accessor accessor2;
    tinygltf::Accessor accessor3;
    tinygltf::Image img;
    tinygltf::Texture texture1;
    tinygltf::Sampler sampler1;
    tinygltf::Asset asset;

    // clang-format off
    float positions[] {
        -0.5F, +0.0F, +0.0F,
        +0.0F, +0.5F, +0.0F,
        +0.5F, +0.0F, +0.0F,
    };

    float texpos[] {
        0.0F, 0.0F,
        0.5F, 1.0F,
        1.0F, 0.0F,
    };

    uint16_t indices[] {0,1,2};

    for (auto index : indices) {
        auto* data = reinterpret_cast<uint8_t*>(&index);
        buffer.data.emplace_back(data[0]);
        buffer.data.emplace_back(data[1]);
    }

    for (auto pos : positions) {
        auto* data = reinterpret_cast<uint8_t*>(&pos);
        buffer.data.emplace_back(data[0]);
        buffer.data.emplace_back(data[1]);
        buffer.data.emplace_back(data[2]);
        buffer.data.emplace_back(data[3]);
    }

    for (auto tex : texpos) {
        auto* data = reinterpret_cast<uint8_t*>(&tex);
        buffer.data.emplace_back(data[0]);
        buffer.data.emplace_back(data[1]);
        buffer.data.emplace_back(data[2]);
        buffer.data.emplace_back(data[3]);
    }
    // clang-format on

    {
        std::ofstream trimodel{"_export/model.bin", std::ios::binary | std::ios::trunc};
        trimodel.write(reinterpret_cast<const char*>(buffer.data.data()), buffer.data.size());
        buffer.uri = "model.bin";
        buffer.byte_length = buffer.data.size();
        // kill off the buffer contents since our use-case will 'assume' the file exists and the
        //  file size is as specified
        buffer.data = {};
    }

    constexpr size_t indices_size = sizeof(indices);
    constexpr size_t positions_size = sizeof(positions);
    constexpr size_t texpos_size = sizeof(texpos);

    // "The indices of the vertices (ELEMENT_ARRAY_BUFFER) take up 6 bytes in the
    // start of the buffer.
    bufferView1.buffer = 0;
    bufferView1.byteOffset = 0;
    bufferView1.byteLength = indices_size;
    bufferView1.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

    // The vertices take up 36 bytes (3 vertices * 3 floating points * 4 bytes)
    // at position 8 in the buffer and are of type ARRAY_BUFFER
    bufferView2.buffer = 0;
    bufferView2.byteOffset = indices_size;
    bufferView2.byteLength = positions_size;
    bufferView2.target = TINYGLTF_TARGET_ARRAY_BUFFER;

    bufferView3.buffer = 0;
    bufferView3.byteOffset = indices_size + positions_size;
    bufferView3.byteLength = texpos_size;
    bufferView3.target = TINYGLTF_TARGET_ARRAY_BUFFER;

    // Describe the layout of bufferView1, the indices of the vertices
    accessor1.bufferView = 0;
    accessor1.byteOffset = 0;
    accessor1.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
    accessor1.count = 3;
    accessor1.type = TINYGLTF_TYPE_SCALAR;
    accessor1.maxValues.push_back(2);
    accessor1.minValues.push_back(0);

    // Describe the layout of bufferView2, the vertices themself
    accessor2.bufferView = 1;
    accessor2.byteOffset = 0;
    accessor2.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    accessor2.count = 3;
    accessor2.type = TINYGLTF_TYPE_VEC3;
    accessor2.maxValues = {1.0, 1.0, 0.0};
    accessor2.minValues = {0.0, 0.0, 0.0};

    // texture positions
    accessor3.bufferView = 2;
    accessor3.byteOffset = 0;
    accessor3.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    accessor3.count = 3;
    accessor3.type = TINYGLTF_TYPE_VEC2;

    // Build the mesh primitive and add it to the mesh
    primitive.indices = 0;                   // The index of the accessor for the vertex indices
    primitive.attributes["POSITION"] = 1;    // The index of the accessor for positions
    primitive.attributes["TEXCOORD_0"] = 2;  // The index of the accessor for texture coordinates
    primitive.material = 0;
    primitive.mode = TINYGLTF_MODE_TRIANGLES;
    mesh.primitives.push_back(primitive);

    img.uri = "sample.png";
    texture1.source = 0;

    // create a triangle node
    node1.mesh = 0;
    node1.translation = {-50.0F, 0.0F, 50.0F};
    node1.scale = {5.0F, 5.0F, 5.0F};

    // create another triangle node and position it
    node2.mesh = 0;
    node2.translation = {+50.0F, 0.0F, -15.0F};
    node2.scale = {8.0F, 8.0F, 8.0F};

    scene.nodes.push_back(0);  // Default scene

    // Define the asset. The version is required
    asset.version = "2.0";
    asset.generator = "tinygltf";

    // Now all that remains is to tie back all the loose objects into the
    // our single model.
    m.scenes.push_back(scene);
    m.meshes.push_back(mesh);
    m.nodes.push_back(node1);
    m.nodes.push_back(node2);
    m.buffers.push_back(buffer);
    m.bufferViews.push_back(bufferView1);
    m.bufferViews.push_back(bufferView2);
    m.bufferViews.push_back(bufferView3);
    m.accessors.push_back(accessor1);
    m.accessors.push_back(accessor2);
    m.accessors.push_back(accessor3);
    m.images.push_back(img);
    m.textures.push_back(texture1);
    m.asset = asset;

    tinygltf::Material mat;
    mat.pbrMetallicRoughness.baseColorFactor = {1.0F, 1.0F, 1.0F, 1.0F};
    mat.pbrMetallicRoughness.baseColorTexture.texCoord = 0;
    mat.pbrMetallicRoughness.baseColorTexture.index = 0;
    mat.doubleSided = true;
    m.materials.push_back(mat);

    // Save it to a file
    tinygltf::TinyGLTF gltf;

    // TODO: Both loading and exporting break when trying to export this since it opens the file to
    //  write 0 bytes since the buffer.data is empty. We just want it to assume the file exists and
    //  the content size is as specified. So a change is needed to Buffer and to SerializeGltfBuffer@7515
    //
    //  if (buffer.byte_length == 0) {
    //    if (!SerializeGltfBufferData(buffer.data, binFilename)) return false;
    //    SerializeNumberProperty("byteLength", buffer.data.size(), o);
    //  } else {
    //      SerializeNumberProperty("byteLength", buffer.byte_length, o);
    //  }
    //
    bool ok = gltf.WriteGltfSceneToFile(&m, "_export/triangle.gltf", false, false, true, false);
    if (!ok) {
        LOG_INFO("failed to write exported scene");
    }
}

void load_triangle_gltf() {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;
    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, "_export/triangle.gltf");

    if (!ret) {
        LOG_INFO("failed to load exported scene");
        LOG_INFO(" - {}", err);
        LOG_INFO(" - {}", warn);
    } else {
        LOG_INFO("loaded {} scenes", model.scenes.size());
        LOG_INFO("loaded {} buffers", model.buffers.size());
        LOG_INFO("loaded {} textures", model.textures.size());
    }
}

}  // namespace