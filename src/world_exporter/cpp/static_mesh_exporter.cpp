//
// Date       : 12/03/2026
// Project    : world_exporter
// Author     : -Ry
//

#include "world_exporter/cpp/pch.h"
#include "world_exporter/cpp/world_exporter.h"
#include "world_exporter/cpp/util/static_mesh.h"
#include "world_exporter/cpp/exporter/static_mesh_exporter.h"

#include "tinygltf/tiny_gltf.h"

namespace world_exporter {

using namespace helpers;

namespace {
constexpr int invalid_id_v = -1;

struct GltfMeshExportInfo {
    int mesh_buffer_id{invalid_id_v};
    int indices_bv_id{invalid_id_v};
    int pos_ac_id{invalid_id_v};
    int colour_ac_id{invalid_id_v};
    int normal_ac_id{invalid_id_v};
    std::array<int, MAX_UV_COUNT> texcoords_ac_ids{};

    GltfMeshExportInfo() {
        texcoords_ac_ids.fill(invalid_id_v);
    }
};

void export_pos_buffer(
    const StaticMeshExportInfo& info,
    tinygltf::Model& model,
    GltfMeshExportInfo& out_info
);

void export_colour_buffer(
    const StaticMeshExportInfo& info,
    tinygltf::Model& model,
    GltfMeshExportInfo& out_info
);

void export_uvnormal_buffer(
    const StaticMeshExportInfo& info,
    tinygltf::Model& model,
    GltfMeshExportInfo& out_info
);

void export_primitives(
    const StaticMeshExportInfo& info,
    tinygltf::Model& model,
    const GltfMeshExportInfo& gltf,
    tinygltf::Mesh& gltf_mesh
);

}  // namespace

void WorldExporter::export_static_meshes() {
    fs::path static_mesh_dir = m_RootDir / "static_meshes";
    fs::create_directory(static_mesh_dir);

    // collect all unique UStaticMesh objects
    const auto* static_mesh_prop = static_mesh_comp_class->find_prop_and_validate<ZObjectProperty>(L"StaticMesh"_fn);
    std::unordered_set<UObject*> static_meshes{};
    for (const auto* obj : m_StaticMeshComponents) {
        auto* mesh = get_property(static_mesh_prop, 0, reinterpret_cast<uintptr_t>(obj));
        if (mesh != nullptr) {
            static_meshes.insert(mesh);
        }
    }

    StaticMeshExporter exporter{};

    for (auto* obj : static_meshes) {
        auto* mesh = reinterpret_cast<UStaticMesh*>(obj);

        // exporting failed so just skip this mesh
        if (!exporter.export_static_mesh(mesh)) {
            m_MeshMap[reinterpret_cast<uintptr_t>(mesh)] = invalid_id_v;
            continue;
        }

        // TODO: need to export primitives based on the indices

        const StaticMeshExportInfo& info = exporter.export_info();
        GltfMeshExportInfo gltf_export{};

        {  // create the raw buffer
            gltf_export.mesh_buffer_id = static_cast<int>(m_TheModel.buffers.size());
            tinygltf::Buffer& mesh_data = m_TheModel.buffers.emplace_back();
            mesh_data.data = info.binary_buffer;
            mesh_data.name = obj->Name();
        }

        {  // index buffer
            gltf_export.indices_bv_id = static_cast<int>(m_TheModel.bufferViews.size());
            tinygltf::BufferView& indices_bv = m_TheModel.bufferViews.emplace_back();
            indices_bv.buffer = gltf_export.mesh_buffer_id;
            indices_bv.byteOffset = info.index_buffer.start;
            indices_bv.byteLength = info.index_buffer.size();
            indices_bv.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
        }

        export_pos_buffer(info, m_TheModel, gltf_export);
        if (info.colour_buffer) {
            export_colour_buffer(info, m_TheModel, gltf_export);
        }
        export_uvnormal_buffer(info, m_TheModel, gltf_export);

        // create the mesh
        int mesh_id = static_cast<int>(m_TheModel.meshes.size());
        m_MeshMap[reinterpret_cast<uintptr_t>(obj)] = mesh_id;
        tinygltf::Mesh& gltf_mesh = m_TheModel.meshes.emplace_back();
        gltf_mesh.name = obj->Name();
        export_primitives(info, m_TheModel, gltf_export, gltf_mesh);
    }
}

void WorldExporter::export_static_mesh(const fs::path& /*mesh_path*/, helpers::UStaticMesh* /*mesh*/) {
}

namespace {

void export_pos_buffer(
    const StaticMeshExportInfo& info,
    tinygltf::Model& model,
    GltfMeshExportInfo& out_info
) {
    int pos_bv_id = static_cast<int>(model.bufferViews.size());
    tinygltf::BufferView& pos_bv = model.bufferViews.emplace_back();
    pos_bv.buffer = out_info.mesh_buffer_id;
    pos_bv.byteOffset = info.position_buffer.start;
    pos_bv.byteLength = info.position_buffer.size();
    pos_bv.target = TINYGLTF_TARGET_ARRAY_BUFFER;

    out_info.pos_ac_id = static_cast<int>(model.accessors.size());
    tinygltf::Accessor& pos_ac = model.accessors.emplace_back();
    pos_ac.bufferView = pos_bv_id;
    pos_ac.byteOffset = 0;
    pos_ac.count = info.position_buffer.vertex_count;
    pos_ac.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    pos_ac.type = TINYGLTF_TYPE_VEC3;
}

void export_colour_buffer(
    const StaticMeshExportInfo& info,
    tinygltf::Model& model,
    GltfMeshExportInfo& out_info
) {
    int colour_bv_id = static_cast<int>(model.bufferViews.size());
    tinygltf::BufferView& colour_bv = model.bufferViews.emplace_back();
    colour_bv.buffer = out_info.mesh_buffer_id;
    colour_bv.byteOffset = info.colour_buffer.start;
    colour_bv.byteLength = info.colour_buffer.size();
    colour_bv.target = TINYGLTF_TARGET_ARRAY_BUFFER;

    out_info.colour_ac_id = static_cast<int>(model.accessors.size());
    tinygltf::Accessor& colour_ac = model.accessors.emplace_back();
    colour_ac.bufferView = colour_bv_id;
    colour_ac.byteOffset = 0;
    colour_ac.count = info.colour_buffer.vertex_count;
    colour_ac.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
    colour_ac.type = TINYGLTF_TYPE_VEC4;
    colour_ac.normalized = true;
}

void export_uvnormal_buffer(
    const StaticMeshExportInfo& info,
    tinygltf::Model& model,
    GltfMeshExportInfo& out_info
) {
    int uvnormal_bv_id = static_cast<int>(model.bufferViews.size());
    tinygltf::BufferView& uvnormal_bv = model.bufferViews.emplace_back();
    uvnormal_bv.buffer = out_info.mesh_buffer_id;
    uvnormal_bv.byteOffset = info.uvnormal_buffer.start;
    uvnormal_bv.byteLength = info.uvnormal_buffer.size();
    uvnormal_bv.target = TINYGLTF_TARGET_ARRAY_BUFFER;

    out_info.normal_ac_id = static_cast<int>(model.accessors.size());
    tinygltf::Accessor& normal_ac = model.accessors.emplace_back();
    normal_ac.bufferView = uvnormal_bv_id;
    normal_ac.byteOffset = 0;
    normal_ac.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    normal_ac.count = info.uvnormal_buffer.vertex_count;
    normal_ac.type = TINYGLTF_TYPE_VEC3;

    constexpr size_t normal_size = sizeof(float) * 3;
    constexpr size_t uv_size = sizeof(float) * 2;

    for (size_t i = 0; i < info.num_tex_coords; ++i) {
        // NOLINTNEXTLINE(*-pro-bounds-constant-array-index)
        out_info.texcoords_ac_ids[i] = static_cast<int>(model.accessors.size());
        tinygltf::Accessor& uv = model.accessors.emplace_back();
        uv.bufferView = uvnormal_bv_id;
        // interleaved; 12, 20, 28, 36, 44
        uv.byteOffset = normal_size + (uv_size * i);
        uv.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        uv.count = info.uvnormal_buffer.vertex_count;
        uv.type = TINYGLTF_TYPE_VEC2;
    }
}

void export_primitives(
    const StaticMeshExportInfo& info,
    tinygltf::Model& model,
    const GltfMeshExportInfo& gltf_export,
    tinygltf::Mesh& gltf_mesh
) {

    if (info.primitives.empty()) {
      LOG(WARNING, "mesh {} has no primitives?", gltf_mesh.name);
    }

    std::random_device rd{};
    std::mt19937 rng{rd()};
    std::uniform_real_distribution<double> random_real{0.25, 1.0};

    for (const auto& primitive : info.primitives) {
        tinygltf::Primitive& mesh_primitive = gltf_mesh.primitives.emplace_back();

        // TODO: material extraction - currently just filling it with random materials
        mesh_primitive.material = static_cast<int>(model.materials.size());
        tinygltf::Material& material = model.materials.emplace_back();
        material.pbrMetallicRoughness.baseColorFactor = {
            random_real(rng),
            random_real(rng),
            random_real(rng),
            1.0
        };
        material.pbrMetallicRoughness.metallicFactor = random_real(rng);
        material.pbrMetallicRoughness.roughnessFactor = random_real(rng);

        mesh_primitive.mode = TINYGLTF_MODE_TRIANGLES;
        mesh_primitive.indices = static_cast<int>(model.accessors.size());

        tinygltf::Accessor& indices_ac = model.accessors.emplace_back();
        indices_ac.bufferView = gltf_export.indices_bv_id;
        indices_ac.byteOffset = primitive.indices.start;
        indices_ac.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
        indices_ac.count = primitive.indices.vertex_count;
        indices_ac.type = TINYGLTF_TYPE_SCALAR;

        mesh_primitive.attributes["POSITION"] = gltf_export.pos_ac_id;
        if (gltf_export.colour_ac_id != invalid_id_v) {
            mesh_primitive.attributes["COLOR_0"] = gltf_export.colour_ac_id;
        }
        mesh_primitive.attributes["NORMAL"] = gltf_export.normal_ac_id;
        for (size_t i = 0; i < gltf_export.texcoords_ac_ids.size(); ++i) {
            // NOLINTNEXTLINE(*-pro-bounds-constant-array-index)
            int id = gltf_export.texcoords_ac_ids[i];
            if (id != invalid_id_v) {
                mesh_primitive.attributes[std::format("TEXCOORD_{}", i)] = id;
            }
        }
    }
}

}  // namespace

}  // namespace world_exporter
