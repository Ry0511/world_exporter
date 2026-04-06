//
// Date       : 04/04/2026
// Project    : world_exporter
// Author     : -Ry
//

#include "world_exporter/cpp/exporter/material_exporter.h"
#include "unrealsdk/unreal/find_class.h"
#include "unrealsdk/unreal/properties/zboolproperty.h"

namespace world_exporter {

namespace {
using namespace unrealsdk;
using namespace unrealsdk::unreal;
using namespace helpers;
}  // namespace

bool MaterialExporter::export_material(UObject* obj) {
    if (obj == nullptr) {
        LOG(WARNING, "can not export null material");
        return false;
    }

    static auto* mat_inst_cls = find_class(L"MaterialInstanceConstant"_fn);
    if (!obj->is_instance(mat_inst_cls)) {
        LOG(WARNING, "material is not a material instance constant; {}", obj->Class()->Name());
        return false;
    }

    // var() const array<TextureParameterValue> TextureParameterValues;
    static auto* prop_texture_parameters = mat_inst_cls->find_prop_and_validate<ZArrayProperty>(L"TextureParameterValues"_fn);
    static FName name_diffuse = L"p_Diffuse"_fn;
    static FName name_normal = L"p_Normal"_fn;

    TextureExporter texture_exporter{};
    WrappedArray texture_params = get_property(prop_texture_parameters, 0, reinterpret_cast<uintptr_t>(obj));

    for (size_t i = 0; i < texture_params.size(); ++i) {
        WrappedStruct tex_param = texture_params.get_at<ZStructProperty>(i);

        // quick check that the texture is valid/sane
        UObject* texture = tex_param.get<ZObjectProperty>(L"ParameterValue"_fn);
        if (texture == nullptr) {
            continue;
        }

        // pull the texture directly from the parameters
        FName parameter_name = tex_param.get<ZNameProperty>(L"ParameterName"_fn);
        if (parameter_name == name_diffuse) {
            if (texture_exporter.export_texture(texture)) {
                m_Export.diffuse_texture = std::move(texture_exporter.export_info());
            } else {
                LOG(ERROR, "failed to export diffuse texture; {}", obj->get_path_name());
            }
        } else if (parameter_name == name_normal) {
            if (texture_exporter.export_texture(texture)) {
                m_Export.normal_texture = std::move(texture_exporter.export_info());
            } else {
                LOG(ERROR, "failed to export normal texture; {}", obj->get_path_name());
            }
        }
    }

    return m_Export;
}

}  // namespace world_exporter
