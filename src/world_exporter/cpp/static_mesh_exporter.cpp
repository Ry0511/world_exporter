//
// Date       : 12/03/2026
// Project    : world_exporter
// Author     : -Ry
//

#include "world_exporter/cpp/world_exporter.h"
#include "world_exporter/cpp/util/static_mesh.h"
#include "world_exporter/cpp/exporter/static_mesh_exporter.h"

#include "tinygltf/tiny_gltf.h"

namespace world_exporter {

using namespace helpers;

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
            m_MeshMap[reinterpret_cast<uintptr_t>(mesh)] = -1;
            continue;
        }

        const StaticMeshExportInfo& info = exporter.export_info();
        std::string mesh_name = unrealsdk::utils::narrow(obj->get_path_name());
        std::ranges::transform(mesh_name.begin(), mesh_name.end(), mesh_name.begin(), ::tolower);
        mesh_name += ".bin";

        // TODO: maybe a good idea to split this out into helper functions

        int mesh_buffer_id = static_cast<int>(m_TheModel.buffers.size());
        int indices_ac_id = -1;
        int pos_ac_id = -1;
        int colour_ac_id = -1;
        std::vector<int> uvnormal_ac_ids{};

        { // do not want to use this reference outside of this scope
            tinygltf::Buffer& mesh_data = m_TheModel.buffers.emplace_back();
            mesh_data.data = info.binary_buffer;
            mesh_data.name = obj->Name();
            // mesh_data.uri = fs::relative(m_RootDir, static_mesh_dir / mesh_name).string();
        }

        { // index buffer
            int indices_bv_id = static_cast<int>(m_TheModel.bufferViews.size());
            tinygltf::BufferView& indices_bv = m_TheModel.bufferViews.emplace_back();
            indices_bv.buffer = mesh_buffer_id;
            indices_bv.byteOffset = info.index_buffer.start;
            indices_bv.byteLength = info.index_buffer.size();
            indices_bv.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

            indices_ac_id = static_cast<int>(m_TheModel.accessors.size());
            tinygltf::Accessor& indices_ac = m_TheModel.accessors.emplace_back();
            indices_ac.bufferView = indices_bv_id;
            indices_ac.byteOffset = 0;
            indices_ac.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
            indices_ac.count = info.index_buffer.vertex_count;
            indices_ac.type = TINYGLTF_TYPE_SCALAR;
        }

        { // vec3f position buffer
            int pos_bv_id = static_cast<int>(m_TheModel.bufferViews.size());
            tinygltf::BufferView& pos_bv = m_TheModel.bufferViews.emplace_back();
            pos_bv.buffer = mesh_buffer_id;
            pos_bv.byteOffset = info.position_buffer.start;
            pos_bv.byteLength = info.position_buffer.size();
            pos_bv.target = TINYGLTF_TARGET_ARRAY_BUFFER;

            pos_ac_id = static_cast<int>(m_TheModel.accessors.size());
            tinygltf::Accessor& pos_ac = m_TheModel.accessors.emplace_back();
            pos_ac.bufferView = pos_bv_id;
            pos_ac.byteOffset = 0;
            pos_ac.count = info.position_buffer.vertex_count;
            pos_ac.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            pos_ac.type = TINYGLTF_TYPE_VEC3;
        }

        // optional colour buffer
        if (info.colour_buffer) {
            int colour_bv_id = static_cast<int>(m_TheModel.bufferViews.size());
            tinygltf::BufferView& colour_bv = m_TheModel.bufferViews.emplace_back();
            colour_bv.buffer = mesh_buffer_id;
            colour_bv.byteOffset = info.colour_buffer.start;
            colour_bv.byteLength = info.colour_buffer.size();
            colour_bv.target = TINYGLTF_TARGET_ARRAY_BUFFER;

            colour_ac_id = static_cast<int>(m_TheModel.accessors.size());
            tinygltf::Accessor& colour_ac = m_TheModel.accessors.emplace_back();
            colour_ac.bufferView = colour_bv_id;
            colour_ac.byteOffset = 0;
            colour_ac.count = info.colour_buffer.vertex_count;
            colour_ac.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
            colour_ac.type = TINYGLTF_TYPE_VEC4;
            colour_ac.normalized = true;
        }

        // uv/normal buffer
        {
            int uvnormal_bv_id = static_cast<int>(m_TheModel.bufferViews.size());
            tinygltf::BufferView& uvnormal_bv = m_TheModel.bufferViews.emplace_back();
            uvnormal_bv.buffer = mesh_buffer_id;
            uvnormal_bv.byteOffset = info.uvnormal_buffer.start;
            uvnormal_bv.byteLength = info.uvnormal_buffer.size();
            uvnormal_bv.target = TINYGLTF_TARGET_ARRAY_BUFFER;

            {
                uvnormal_ac_ids.emplace_back(static_cast<int>(m_TheModel.accessors.size()));
                tinygltf::Accessor& normal_ac = m_TheModel.accessors.emplace_back();
                normal_ac.bufferView = uvnormal_bv_id;
                normal_ac.byteOffset = 0;
                normal_ac.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                normal_ac.count = info.uvnormal_buffer.vertex_count;
                normal_ac.type = TINYGLTF_TYPE_VEC3;
            }

            constexpr size_t normal_size = sizeof(float) * 3;
            constexpr size_t uv_size = sizeof(float) * 2;

            for (size_t i = 0; i < info.num_tex_coords; ++i) {
                uvnormal_ac_ids.emplace_back(static_cast<int>(m_TheModel.accessors.size()));
                tinygltf::Accessor& uv = m_TheModel.accessors.emplace_back();
                uv.bufferView = uvnormal_bv_id;
                // interleaved; 12, 20, 28, 36, 44
                uv.byteOffset = normal_size + (uv_size * i);
                uv.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
                uv.count = info.uvnormal_buffer.vertex_count;
                uv.type = TINYGLTF_TYPE_VEC2;
            }
        }

        int gltf_mesh_id = static_cast<int>(m_TheModel.meshes.size());
        m_MeshMap[reinterpret_cast<uintptr_t>(obj)] = gltf_mesh_id;
        tinygltf::Mesh& gltf_mesh = m_TheModel.meshes.emplace_back();
        gltf_mesh.name = obj->Name();

        // single primitive currently
        tinygltf::Primitive& mesh_primitive = gltf_mesh.primitives.emplace_back();
        mesh_primitive.material = 0;
        mesh_primitive.mode = TINYGLTF_MODE_TRIANGLES;
        mesh_primitive.indices = indices_ac_id;
        mesh_primitive.attributes["POSITION"] = pos_ac_id;
        if (colour_ac_id != -1) {
            mesh_primitive.attributes["COLOR_0"] = colour_ac_id;
        }
        mesh_primitive.attributes["NORMAL"] = uvnormal_ac_ids.front();
        for (size_t i = 1; i < uvnormal_ac_ids.size(); ++i) {
            mesh_primitive.attributes["TEXCOORD_" + std::to_string(i-1)] = uvnormal_ac_ids[i];
        }
    }
}

void WorldExporter::export_static_mesh(const fs::path& /*mesh_path*/, helpers::UStaticMesh* /*mesh*/) {
}

}  // namespace world_exporter
