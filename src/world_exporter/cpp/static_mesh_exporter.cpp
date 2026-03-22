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

namespace {
struct ExportInfo {
    tinygltf::Model* model;
    tinygltf::Primitive* primitive;
    std::ofstream* out;
    size_t offset;
};

void export_index_buffer(ExportInfo& info, const FRawStaticIndexBuffer& buf);
void export_position_buffer(ExportInfo& info, const FPositionVertexBuffer& buf);
void export_colour_buffer(ExportInfo& info, const FColourVertexBuffer& buf);
void export_uvnormal_buffer(ExportInfo& info, const FStaticMeshVertexBuffer& buf);
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

    auto* model = mesh->LodModels.at(0);

    // just as a first pass sanity check
    if (
        model->IndexBuffer.Indices.data == nullptr
        || model->PositionVertexBuffer.VertexData == nullptr
        || model->VertexBuffer.VertexData == nullptr
        // not a guarantee that the vertex data is triangulated but should avoid a crash
        || (model->IndexBuffer.Indices.size() % 3) != 0
    ) {
        LOG(INFO, "LOD[0] does not seem to be setup/ready for {} - skipping", mesh->Name);
        return;
    }

    tinygltf::Buffer buffer{};
    tinygltf::Primitive primitive{};
    primitive.material = 0;
    primitive.mode = TINYGLTF_MODE_TRIANGLES;
    buffer.uri = fs::relative(mesh_path, m_RootDir).string();

    std::ofstream out{mesh_path, std::ios::binary | std::ios::trunc};

    ExportInfo export_info{
        .model = &m_TheModel,
        .primitive = &primitive,
        .out = &out,
        .offset = 0,
    };

    // TODO: this approach works for single-mesh models but multi-mesh/multi-material models will
    //  require multiple primitives as they bind to a range of indices.
    export_index_buffer(export_info, model->IndexBuffer);
    export_position_buffer(export_info, model->PositionVertexBuffer);
    export_colour_buffer(export_info, model->ColourVertexBuffer);
    export_uvnormal_buffer(export_info, model->VertexBuffer);

    tinygltf::Mesh the_mesh{};
    the_mesh.name = std::string{mesh->Name};
    the_mesh.primitives.push_back(primitive);
    m_MeshMap[reinterpret_cast<uintptr_t>(mesh)] = static_cast<int>(m_TheModel.meshes.size());
    m_TheModel.meshes.push_back(the_mesh);

    buffer.byte_length = export_info.offset;
    m_TheModel.buffers.push_back(buffer);
}

namespace {

void export_index_buffer(ExportInfo& info, const FRawStaticIndexBuffer& buf) {
    const auto& indices = buf.Indices;
    const auto size_in_bytes = indices.size() * sizeof(uint16_t);
    for (size_t i = 0; i < indices.size(); i += 3) {
        //      1
        //     . .
        //    0...2
        //
        //      2
        //     . .
        //    0...1
        info.out->write(reinterpret_cast<char*>(indices.data + i + 0), sizeof(uint16_t));
        info.out->write(reinterpret_cast<char*>(indices.data + i + 2), sizeof(uint16_t));
        info.out->write(reinterpret_cast<char*>(indices.data + i + 1), sizeof(uint16_t));
    }

    tinygltf::BufferView view{};
    view.buffer = static_cast<int>(info.model->buffers.size());
    view.byteOffset = info.offset;
    view.byteLength = size_in_bytes;
    view.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
    info.offset = info.offset + size_in_bytes;

    auto access = tinygltf::Accessor{};
    access.bufferView = static_cast<int>(info.model->bufferViews.size());
    access.byteOffset = 0;
    access.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
    access.count = indices.size();
    access.type = TINYGLTF_TYPE_SCALAR;

    info.primitive->indices = static_cast<int>(info.model->accessors.size());

    info.model->bufferViews.push_back(view);
    info.model->accessors.push_back(access);
}

void export_position_buffer(ExportInfo& info, const FPositionVertexBuffer& buf) {
    auto data = buf.VertexData->data();
    auto stride = buf.VertexData->stride();
    size_t size_in_bytes = buf.NumVertices * sizeof(FVector);
    for (size_t i = 0; i < buf.NumVertices; ++i) {
        const auto& pos = *reinterpret_cast<FVector*>(data + (i * stride));
        FVector p{-(pos.Y * 0.01F), pos.Z * 0.01F, pos.X * 0.01F};
        info.out->write(reinterpret_cast<char*>(&p), sizeof(FVector));
    }

    tinygltf::BufferView view{};
    view.buffer = static_cast<int>(info.model->buffers.size());
    view.byteOffset = info.offset;
    view.byteLength = size_in_bytes;
    view.target = TINYGLTF_TARGET_ARRAY_BUFFER;
    info.offset = info.offset + size_in_bytes;

    tinygltf::Accessor access{};
    access.bufferView = static_cast<int>(info.model->bufferViews.size());
    access.byteOffset = 0;
    access.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    access.count = buf.NumVertices;
    access.type = TINYGLTF_TYPE_VEC3;

    info.primitive->attributes["POSITION"] = static_cast<int>(info.model->accessors.size());

    info.model->bufferViews.push_back(view);
    info.model->accessors.push_back(access);
}

void export_colour_buffer(ExportInfo& info, const FColourVertexBuffer& buf) {
    // TODO: null check here is required to avoid a crash - might need this in more places
    if (buf.bIsInitialised == 0 || buf.VertexData == nullptr) {
        return;
    }

    uint8_t* data = buf.VertexData->data();
    auto stride = buf.VertexData->stride();

    if (stride != sizeof(FColor)) {
        LOG(WARNING, "colour buffer does not fit into FColor");
        return;
    }

    size_t size_in_bytes = buf.NumVertices * sizeof(FColor);

    for (size_t i = 0; i < buf.NumVertices; ++i) {
        const auto& col = *reinterpret_cast<FColor*>(data + (i * stride));
        uint8_t rgba[]{col.R, col.G, col.B, col.A};
        info.out->write(reinterpret_cast<const char*>(&rgba[0]), sizeof(FColor));
    }

    tinygltf::BufferView view{};
    view.buffer = static_cast<int>(info.model->buffers.size());
    view.byteOffset = info.offset;
    view.byteLength = size_in_bytes;
    view.target = TINYGLTF_TARGET_ARRAY_BUFFER;
    info.offset = info.offset + size_in_bytes;

    tinygltf::Accessor access{};
    access.bufferView = static_cast<int>(info.model->bufferViews.size());
    access.byteOffset = 0;
    access.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
    access.count = buf.NumVertices;
    access.type = TINYGLTF_TYPE_VEC4;
    access.normalized = true;
    info.primitive->attributes["COLOR_0"] = static_cast<int>(info.model->accessors.size());

    info.offset = info.offset + size_in_bytes;
    info.model->accessors.push_back(access);
    info.model->bufferViews.push_back(view);
}

void export_uvnormal_buffer(ExportInfo& info, const FStaticMeshVertexBuffer& buf) {
    // write all uvs
    if (buf.bUseFullPrecisionUVs == 0) {
        return;
    }
    auto* data = buf.VertexData->data();
    auto stride = buf.VertexData->stride();

    size_t size_in_bytes{0};

    for (size_t i = 0; i < buf.NumVertices; ++i) {
        auto* vert = reinterpret_cast<TStaticMeshFullVertexFloat16UVs*>(data + (i * stride));

        // TODO: need to implement this one
        FVector norm{0.0F, 0.0F, 0.0F};
        info.out->write(reinterpret_cast<char*>(&norm), sizeof(FVector));

        for (size_t j = 0; j < buf.NumTexCoords; ++j) {
            // NOLINTNEXTLINE(*-pro-bounds-constant-array-index)
            FVector2D uv = vert->UVs[j].as_vec2();
            info.out->write(reinterpret_cast<char*>(&uv), sizeof(FVector2D));
        }
        size_in_bytes += sizeof(FVector) + (sizeof(FVector2D) * buf.NumTexCoords);
    }

    tinygltf::BufferView view{};
    view.buffer = static_cast<int>(info.model->buffers.size());
    view.byteOffset = info.offset;
    view.byteLength = size_in_bytes;
    view.target = TINYGLTF_TARGET_ARRAY_BUFFER;
    info.offset = info.offset + size_in_bytes;

    tinygltf::Accessor acc_normal{};
    acc_normal.bufferView = static_cast<int>(info.model->bufferViews.size());
    acc_normal.byteOffset = 0;
    acc_normal.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    acc_normal.count = buf.NumVertices;
    acc_normal.type = TINYGLTF_TYPE_VEC3;

    // https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#meshes
    info.primitive->attributes["NORMAL"] = static_cast<int>(info.model->accessors.size());
    info.model->accessors.push_back(acc_normal);

    size_t uv_offset{sizeof(FVector)};
    for (size_t i = 0; i < buf.NumTexCoords; ++i) {
        tinygltf::Accessor uv{};
        uv.bufferView = acc_normal.bufferView;
        uv.byteOffset = uv_offset;
        uv.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        uv.count = buf.NumVertices;
        uv.type = TINYGLTF_TYPE_VEC2;
        uv_offset += sizeof(FVector2D);

        info.primitive->attributes["TEXCOORD_" + std::to_string(i)] = static_cast<int>(info.model->accessors.size());
        info.model->accessors.push_back(uv);
    }
    info.model->bufferViews.push_back(view);
}

}  // namespace

}  // namespace world_exporter
