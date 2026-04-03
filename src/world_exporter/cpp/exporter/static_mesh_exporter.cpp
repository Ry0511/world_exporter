//
// Date       : 30/03/2026
// Project    : world_exporter
// Author     : -Ry
//

#include "world_exporter/cpp/pch.h"

#include "world_exporter/cpp/exporter/static_mesh_exporter.h"
#include "world_exporter/cpp/util/static_mesh.h"

namespace world_exporter {

////////////////////////////////////////////////////////////////////////////////
// | INTERNAL HELPERS |
////////////////////////////////////////////////////////////////////////////////

namespace {
constexpr size_t buffer_prealloc_size{1024LLU * 1024LLU};
constexpr size_t primitives_prealloc_count{16};
// UE3 1.0 ~= 1cm, glTF 1.0 ~= 1m
constexpr glm::vec3 gltf_unit_scale{0.01F, 0.01F, 0.01F};

glm::vec3 gltf_swizzle(const float* vec) noexcept {
    // glTF -X = Right   <> ( UE +Y = Right   ) ; X = -Y
    //      +Y = Up      <> ( UE +Z = Up      ) ; Y = +Z
    //      +Z = Forward <> ( UE +X = Forward ) ; Z = +X
    // see: https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#coordinate-system-and-units
    return glm::vec3{-vec[1], vec[2], vec[0]};
}
}  // namespace

using namespace helpers;

////////////////////////////////////////////////////////////////////////////////
// | INITIALISATION |
////////////////////////////////////////////////////////////////////////////////

void StaticMeshExportInfo::reset() noexcept {
    binary_buffer.clear();
    primitives.clear();
    index_buffer = {};
    position_buffer = {};
    colour_buffer = {};
    uvnormal_buffer = {};
    num_tex_coords = 0;
}

void StaticMeshExporter::reset() {
    m_ExportInfo.reset();
    m_ExportInfo.binary_buffer.reserve(buffer_prealloc_size);
    m_ExportInfo.primitives.reserve(primitives_prealloc_count);
}

bool StaticMeshExporter::export_static_mesh(UStaticMesh* mesh) {
    reset();
    auto* obj = reinterpret_cast<unrealsdk::unreal::UObject*>(mesh);

    if (mesh == nullptr) {
        LOG(WARNING, "can not export null static mesh");
        return false;
    }

    if (mesh->LodModels.data == nullptr || mesh->LodModels.count <= 0) {
        LOG(ERROR, "error exporting static mesh; {}", obj->get_path_name());
        LOG(ERROR, "can not export static mesh because there are not LOD models");
        return false;
    }

    FStaticMeshRenderData* model = mesh->LodModels.at(0);

    if (model == nullptr) {
        LOG(ERROR, "LOD[0] model is null?");
        return false;
    }

    try {
        export_index_buffer(*model);
        export_position_buffer(*model);
        export_colour_buffer(*model);
        export_uvnormal_buffer(*model);

        // just being extra pedantic here
        if (model->SubMeshes.data != nullptr) {
            for (size_t i = 0; i < model->SubMeshes.size(); ++i) {
                const auto& sub_mesh = model->SubMeshes[i];
                auto& primitive = m_ExportInfo.primitives.emplace_back();
                primitive.material = sub_mesh.Material;
                size_t offset = sub_mesh.FirstIndex * sizeof(uint16_t);
                size_t end = (sub_mesh.NumTriangles * uint32_t{3}) * sizeof(uint16_t);
                primitive.indices = {offset, end, sub_mesh.NumTriangles * uint32_t{3}};
            }
        }
    } catch (const std::runtime_error& err) {
        LOG(ERROR, "error exporting static mesh; {}", obj->get_path_name());
        LOG(ERROR, "  - {}", err.what());
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
// | BUFFER EXPORTING |
////////////////////////////////////////////////////////////////////////////////

void StaticMeshExporter::insert_range_as_bytes(const uint8_t* data, size_t count) noexcept {
    auto& buf = m_ExportInfo.binary_buffer;
    buf.insert(buf.end(), data, data + count);
}

void StaticMeshExporter::export_index_buffer(const FStaticMeshRenderData& mesh) {
    const FRawStaticIndexBuffer& buf = mesh.IndexBuffer;

    // general sanity checks
    if (buf.Indices.data == nullptr || buf.Indices.size() <= 0) {
        throw std::runtime_error{"failed to export index buffer"};
    }

    // fairly certain this will never be true but if it does happen I would rather not export garbage
    if ((buf.Indices.size() % 3) != 0) {
        throw std::runtime_error{"index buffer is not a multiple of 3 ~ not triangulated?"};
    }

    m_ExportInfo.index_buffer.start = m_ExportInfo.binary_buffer.size();
    // a later flip to the vertex positions should flip the winding order but not exactly sure if it does
    insert_range_as_bytes(reinterpret_cast<const uint8_t*>(buf.Indices.data), buf.Indices.size() * sizeof(int16_t));
    m_ExportInfo.index_buffer.end = m_ExportInfo.binary_buffer.size();
    m_ExportInfo.index_buffer.vertex_count = buf.Indices.size();
}

void StaticMeshExporter::export_position_buffer(const helpers::FStaticMeshRenderData& mesh) {
    const FPositionVertexBuffer& buf = mesh.PositionVertexBuffer;

    if (buf.VertexData == nullptr) {
        throw std::runtime_error{"failed to export position buffer"};
    }

    uint8_t* data = buf.VertexData->data();

    if (data == nullptr) {
        throw std::runtime_error{"failed to obtain position vertex data pointer"};
    }

    uint32_t stride = buf.VertexData->stride();

    if (stride != sizeof(FVector)) {
        throw std::runtime_error{"unexpected stride in position buffer"};
    }

    m_ExportInfo.position_buffer.start = m_ExportInfo.binary_buffer.size();
    for (size_t i = 0; i < buf.NumVertices; ++i) {
        const auto* pos = reinterpret_cast<const float*>(data + (i * stride));
        // is the unit re-scale needed here?
        glm::vec3 gltf = gltf_swizzle(pos) * gltf_unit_scale;
        insert_range_as_bytes(
            reinterpret_cast<const uint8_t*>(glm::value_ptr(gltf)),
            sizeof(glm::vec3)
        );
    }
    m_ExportInfo.position_buffer.end = m_ExportInfo.binary_buffer.size();
    m_ExportInfo.position_buffer.vertex_count = buf.NumVertices;
}

void StaticMeshExporter::export_colour_buffer(const FStaticMeshRenderData& mesh) {
    const FColourVertexBuffer& buf = mesh.ColourVertexBuffer;

    if (buf.VertexData == nullptr) {
        // colour buffers are optional
        return;
    }

    uint8_t* data = buf.VertexData->data();

    if (data == nullptr) {
        LOG(WARNING, "failed to obtain colour data pointer");
        return;
    }

    auto stride = buf.VertexData->stride();

    if (stride != sizeof(FColor)) {
        LOG(WARNING, "colour buffer does not fit into FColor");
        return;
    }

    m_ExportInfo.colour_buffer.start = m_ExportInfo.binary_buffer.size();
    for (size_t i = 0; i < buf.NumVertices; ++i) {
        const auto& col = *reinterpret_cast<FColor*>(data + (i * stride));
        uint8_t rgba[]{col.R, col.G, col.B, col.A};
        insert_range_as_bytes(&rgba[0], sizeof(rgba));
    }
    m_ExportInfo.colour_buffer.end = m_ExportInfo.binary_buffer.size();
    m_ExportInfo.colour_buffer.vertex_count = buf.NumVertices;
}

void StaticMeshExporter::export_uvnormal_buffer(const FStaticMeshRenderData& mesh) {
    const FStaticMeshVertexBuffer& buf = mesh.VertexBuffer;

    if (buf.bUseFullPrecisionUVs != 0) {
        throw std::runtime_error{"full precision uvs are not supported currently"};
    }

    auto* data = buf.VertexData->data();

    if (data == nullptr) {
        throw std::runtime_error{"failed to obtain uv/normal data pointer"};
    }

    auto stride = buf.VertexData->stride();

    m_ExportInfo.uvnormal_buffer.start = m_ExportInfo.binary_buffer.size();
    m_ExportInfo.num_tex_coords = buf.NumTexCoords;
    for (size_t i = 0; i < buf.NumVertices; ++i) {
        auto* vert = reinterpret_cast<TStaticMeshFullVertexFloat16UVs*>(data + (i * stride));

        // TODO: still need to implement this
        FVector norm{0.0F, 0.0F, 0.0F};
        insert_range_as_bytes(reinterpret_cast<const uint8_t*>(&norm), sizeof(FVector));

        for (size_t j = 0; j < buf.NumTexCoords; ++j) {
            // NOLINTNEXTLINE(*-pro-bounds-constant-array-index)
            FVector2D uv = vert->UVs[j].as_vec2();
            insert_range_as_bytes(reinterpret_cast<const uint8_t*>(&uv), sizeof(FVector2D));
        }
    }
    m_ExportInfo.uvnormal_buffer.end = m_ExportInfo.binary_buffer.size();
    m_ExportInfo.uvnormal_buffer.vertex_count = buf.NumVertices;
}

}  // namespace world_exporter
