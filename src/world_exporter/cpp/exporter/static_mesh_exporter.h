//
// Date       : 30/03/2026
// Project    : world_exporter
// Author     : -Ry
//
#ifndef WORLD_EXPORTER_STATIC_MESH_EXPORTER_H
#define WORLD_EXPORTER_STATIC_MESH_EXPORTER_H

#include "world_exporter/cpp/pch.h"

namespace world_exporter {

namespace helpers {
struct UStaticMesh;
struct FStaticMeshRenderData;
}  // namespace helpers

struct BufferRangeInfo {
    static constexpr size_t invalid_index_v = std::numeric_limits<size_t>::max();
    size_t start{invalid_index_v};
    size_t end{invalid_index_v};  // size() + 1
    size_t vertex_count{0};

    size_t size() const noexcept {
        return end - start;
    }

    operator bool() const noexcept {
        return start != invalid_index_v && end != invalid_index_v;
    }
};

struct IndexedMeshPrimitive {
    void* material{nullptr};    // native unreal pointer to the material for this primitive
    BufferRangeInfo indices{};  // the range of indices for this mesh primitive
};

/**
 * data holder of a meshes vertex data and a series of buffer descriptions outlining where said
 * data can be found. The underlying data is assumed to be in the following formats:
 *   - Position Buffer  ~ packed vec3f
 *   - Colour Buffer    ~ packed uint8_t normalised colour information as RGBA; this is optional
 *   - UV Normal Buffer ~ packed vec3f normal + vec2f[N] texture coordinates
 *   - Index Buffer     ~ packed uint16_t buffer of indices; though a model may have multiple
 *                         primitives which define a range of indices and a material for that range.
 */
struct StaticMeshExportInfo {
    // cpu side buffer containing vertex positions, colours, uvs, and normals
    std::vector<uint8_t> binary_buffer;
    std::vector<IndexedMeshPrimitive> primitives;  // material + range of indices
    BufferRangeInfo index_buffer{};                // uint16_t index buffer
    BufferRangeInfo position_buffer{};             // vec3f binding to POSITION ( mandatory )
    BufferRangeInfo colour_buffer{};               // uint8_t RGBA binding to COLOR_0 ( optional )
    BufferRangeInfo uvnormal_buffer{};             // vec3f NORMAL and vec2f TEXCOORD_0, .. N ( mandatory )
    uint32_t num_tex_coords;

    // it is tempting to define the ranges as size_t and assume the start of the next buffer is the
    // end of the previous. but on the off-chance we ever, for some reason, need to add padding bytes
    // to any of the buffers this defined range will be more suitable.

    size_t num_vertices() const noexcept {
        return index_buffer.size();
    }

    void reset() noexcept;
};

/**
 * Bulk/Iterative exporter for Static Meshes - operates by transforming the unreal side data into
 * an intermediate format that is easier to export to glTF.
 */
class StaticMeshExporter {
   private:
    StaticMeshExportInfo m_ExportInfo;

   public:
    StaticMeshExporter() = default;
    ~StaticMeshExporter() = default;

   public:
    const StaticMeshExportInfo& export_info() const noexcept { return m_ExportInfo; }

   public:
    void reset();

    /**
     * @param mesh the mesh to export
     * @return true if the export was successful or false if it was not
     * @note export results can be obtained via `export_info()`
     * @note all calls immediately invalidate the previous result
     */
    bool export_static_mesh(helpers::UStaticMesh* mesh);

   private:
    void insert_range_as_bytes(const uint8_t* data, size_t count) noexcept;

   private:
    void export_index_buffer(const helpers::FStaticMeshRenderData& mesh);
    void export_position_buffer(const helpers::FStaticMeshRenderData& mesh);
    void export_colour_buffer(const helpers::FStaticMeshRenderData& mesh);
    void export_uvnormal_buffer(const helpers::FStaticMeshRenderData& mesh);
};

}  // namespace world_exporter

#endif
