//
// Date       : 12/03/2026
// Project    : world_exporter
// Author     : -Ry
//

#include "world_exporter/cpp/world_exporter.h"
#include "world_exporter/cpp/helpers.h"

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

    for (auto* mesh : static_meshes) {
        std::string mesh_name = unrealsdk::utils::narrow(mesh->get_path_name());
        std::ranges::transform(mesh_name.begin(), mesh_name.end(), mesh_name.begin(), ::tolower);
        fs::path mesh_path = static_mesh_dir / (mesh_name + ".bin");
        export_static_mesh(mesh_path, reinterpret_cast<UStaticMesh*>(mesh));
    }
}

void WorldExporter::export_static_mesh(const fs::path& mesh_path, helpers::UStaticMesh* mesh) {
    if (mesh->LodInfo.size() == 0) {
        LOG(INFO, "mesh {} has not LOD elements", mesh->Name);
        return;
    }

    // TODO: we can also export the colour buffer not sure how much it is used though

    auto* model = mesh->LodModels.at(0);

    // just as a first pass sanity check
    if (
        model->bNeedsCpuAccess == 0
        || model->IndexBuffer.bIsInitialised == 0
        || model->PositionVertexBuffer.bIsInitialised == 0
        || model->VertexBuffer.bIsInitialised == 0
    ) {
        LOG(INFO, "LOD[0] does not seem to be setup/ready for {} - skipping", mesh->Name);
        return;
    }

    // TODO: maybe or maybe not a good idea to embed the buffer data directly into the gltf file.
    //  Issue there is duplicated data for every .gltf file at the cost of basically zero
    //  dependencies.
    tinygltf::Buffer buffer{};
    tinygltf::Primitive primitive{};
    primitive.material = 0;
    primitive.mode = TINYGLTF_MODE_TRIANGLES;
    buffer.uri = fs::relative(mesh_path, m_RootDir).string();
    size_t buffer_id = m_TheModel.buffers.size();

    std::ofstream out{mesh_path, std::ios::binary | std::ios::trunc};
    size_t offset{0};

    {  // write all indices to the file

        const auto& indices = model->IndexBuffer.Indices;
        const auto size_in_bytes = indices.size() * sizeof(uint16_t);
        out.write(reinterpret_cast<char*>(indices.data), size_in_bytes);

        tinygltf::BufferView view{};
        view.buffer = buffer_id;
        view.byteOffset = offset;
        view.byteLength = size_in_bytes;
        view.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
        offset += size_in_bytes;

        // tell it how the buffer view needs to be interpreted
        auto access = tinygltf::Accessor{};
        access.bufferView = m_TheModel.bufferViews.size();
        access.byteOffset = 0;
        access.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
        access.count = indices.size();
        access.type = TINYGLTF_TYPE_SCALAR;

        primitive.indices = m_TheModel.accessors.size();

        m_TheModel.bufferViews.push_back(view);
        m_TheModel.accessors.push_back(access);
    }

    {  // write all vertices
        auto& buf = model->PositionVertexBuffer;
        auto data = buf.VertexData->data();
        auto stride = buf.VertexData->stride();
        size_t size_in_bytes = buf.NumVertices * sizeof(FVector);
        for (size_t i = 0; i < buf.NumVertices; ++i) {
            char* pos = reinterpret_cast<char*>(data + i * stride);
            out.write(pos, sizeof(FVector));
        }

        tinygltf::BufferView view{};
        view.buffer = buffer_id;
        view.byteOffset = offset;
        view.byteLength = size_in_bytes;
        view.target = TINYGLTF_TARGET_ARRAY_BUFFER;
        offset += size_in_bytes;

        tinygltf::Accessor access{};
        access.bufferView = m_TheModel.bufferViews.size();
        access.byteOffset = 0;
        access.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        access.count = buf.NumVertices;
        access.type = TINYGLTF_TYPE_VEC3;

        primitive.attributes["POSITION"] = m_TheModel.accessors.size();

        m_TheModel.bufferViews.push_back(view);
        m_TheModel.accessors.push_back(access);
    }

    // write all uvs
    if (model->VertexBuffer.bUseFullPrecisionUVs != 0) {
        auto& buf = model->VertexBuffer;
        auto* data = buf.VertexData->data();
        auto stride = buf.VertexData->stride();

        size_t size_in_bytes{0};

        for (size_t i = 0; i < buf.NumVertices; ++i) {
            auto* vert = reinterpret_cast<TStaticMeshFullVertexFloat16UVs*>(data + i * stride);

            // TODO: this one needs validating
            FVector norm{0.0F, 0.0F, 0.0F};
            out.write(reinterpret_cast<char*>(&norm), sizeof(FVector));

            for (size_t j = 0; j < buf.NumTexCoords; ++j) {
                FVector2D uv = vert->UVs[j].as_vec2();
                out.write(reinterpret_cast<char*>(&uv), sizeof(FVector2D));
            }
            size_in_bytes += sizeof(FVector) + (sizeof(FVector2D) * buf.NumTexCoords);
        }

        tinygltf::BufferView view{};
        view.buffer = buffer_id;
        view.byteOffset = offset;
        view.byteLength = size_in_bytes;
        view.target = TINYGLTF_TARGET_ARRAY_BUFFER;
        offset += size_in_bytes;

        tinygltf::Accessor acc_normal{};
        acc_normal.bufferView = m_TheModel.bufferViews.size();
        acc_normal.byteOffset = 0;
        acc_normal.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        acc_normal.count = buf.NumVertices;
        acc_normal.type = TINYGLTF_TYPE_VEC3;

        // https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#meshes
        primitive.attributes["NORMAL"] = m_TheModel.accessors.size();
        m_TheModel.accessors.push_back(acc_normal);

        size_t uv_offset{sizeof(FVector)};
        for (size_t i = 0; i < buf.NumTexCoords; ++i) {
            tinygltf::Accessor uv{};
            uv.bufferView = acc_normal.bufferView;
            uv.byteOffset = uv_offset;
            uv.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            uv.count = buf.NumVertices;
            uv.type = TINYGLTF_TYPE_VEC2;
            uv_offset += sizeof(FVector2D);

            primitive.attributes["TEXCOORD_" + std::to_string(i)] = m_TheModel.accessors.size();
            m_TheModel.accessors.push_back(uv);
        }
        m_TheModel.bufferViews.push_back(view);
    }

    tinygltf::Mesh the_mesh{};
    the_mesh.name = std::string{mesh->Name};
    the_mesh.primitives.push_back(primitive);
    m_MeshMap[reinterpret_cast<uintptr_t>(mesh)] = m_TheModel.meshes.size();
    m_TheModel.meshes.push_back(the_mesh);

    // TODO: this is a hack and maybe we should just embed the buffer data into the gltf instead of
    //  storing it separately
    buffer.byte_length = offset;
    m_TheModel.buffers.push_back(buffer);
}

}  // namespace world_exporter
