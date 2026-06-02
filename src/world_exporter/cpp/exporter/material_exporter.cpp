//
// Date       : 04/04/2026
// Project    : world_exporter
// Author     : -Ry
//

#include "world_exporter/cpp/exporter/material_exporter.h"
#include "unrealsdk/unreal/find_class.h"
#include "unrealsdk/unreal/properties/zboolproperty.h"
#include "unrealsdk/unreal/wrappers/gobjects.h"

namespace world_exporter {

////////////////////////////////////////////////////////////////////////////////
// | HELPERS |
////////////////////////////////////////////////////////////////////////////////

namespace {
using namespace unrealsdk;
using namespace unrealsdk::unreal;
using namespace helpers;

/**
 * Attempts to find for the given material and instance of said material which is constant.
 * @param material The material to search for
 * @return material instance or nullptr if not found
 */
UObject* find_material_instance_constant_usage(UObject* material);

// TODO: define a common material extraction strategy i.e., p_Diffuse is used but not everywhere

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// | MEMBER FUNCTIONS |
////////////////////////////////////////////////////////////////////////////////

bool MaterialExporter::export_material(UObject* obj) {
    if (obj == nullptr) {
        LOG(WARNING, "can not export null material");
        return false;
    }

    static auto* mat_interface_cls = find_class(L"MaterialInterface"_fn);

    static auto* mat_inst_cls = find_class(L"MaterialInstanceConstant"_fn);
    if (obj->is_instance(mat_inst_cls)) {
        extract_material_instance_constant(obj);
    }

    return m_Export;
}

void MaterialExporter::extract_material_interface(unrealsdk::unreal::UObject* /*mat*/) {
}

void MaterialExporter::extract_material_instance_constant(unrealsdk::unreal::UObject* material_instance) {
    static auto* mat_inst_cls = find_class(L"MaterialInstanceConstant"_fn);
    static auto* prop_texture_parameters = mat_inst_cls->find_prop_and_validate<ZArrayProperty>(L"TextureParameterValues"_fn);
    static FName name_diffuse = L"p_Diffuse"_fn;
    static FName name_normal = L"p_Normal"_fn;
    static FName name_emissive = L"p_Emissive"_fn;

    TextureExporter texture_exporter{};
    WrappedArray texture_params = get_property(prop_texture_parameters, 0, reinterpret_cast<uintptr_t>(material_instance));

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
                m_Export.diffuse_texture.to_linear();
            } else {
                LOG(ERROR, "failed to export diffuse texture; {}", material_instance->get_path_name());
            }
        } else if (parameter_name == name_normal) {
            if (texture_exporter.export_texture(texture)) {
                m_Export.normal_texture = std::move(texture_exporter.export_info());
            } else {
                LOG(ERROR, "failed to export normal texture; {}", material_instance->get_path_name());
            }
        } else if (parameter_name == name_emissive) {
            if (texture_exporter.export_texture(texture)) {
                m_Export.emissive_texture = std::move(texture_exporter.export_info());
                m_Export.emissive_texture.to_srgb();
            } else {
                LOG(ERROR, "failed to export emissive texture; {}", material_instance->get_path_name());
            }
        }
    }
}

void MaterialExporter::extract_material_props(unrealsdk::unreal::UObject* mat) {
    static auto* mat_cls = find_class(L"Material"_fn);
    uintptr_t base = reinterpret_cast<uintptr_t>(mat);

    static auto* prop_two_sided = mat_cls->find_prop_and_validate<ZBoolProperty>(L"TwoSided"_fn);
    m_Export.is_double_sided = get_property(prop_two_sided, 0, base);

    static auto* prop_alpha_cutoff = mat_cls->find_prop_and_validate<ZFloatProperty>(L"OpacityMaskClipValue"_fn);
    m_Export.alpha_cutoff = get_property(prop_alpha_cutoff, 0, base);
    // probably not tha useful without:
    //   var ScalarMaterialInput OpacityMask;

    static auto* prop_blend_mode = mat_cls->find_prop_and_validate<ZByteProperty>(L"BlendMode"_fn);
    m_Export.blend_mode = static_cast<BlendMode>(get_property(prop_blend_mode, 0, base));
}

////////////////////////////////////////////////////////////////////////////////
// | HELPERS |
////////////////////////////////////////////////////////////////////////////////

namespace {

UObject* find_material_instance_constant_usage(UObject* material) {
    static auto* mat_cls = find_class(L"Material"_fn);
    static auto* mat_iface_cls = find_class(L"MaterialInterface"_fn);
    static auto* fn_get_material = mat_iface_cls->find_func_and_validate(L"GetMaterial"_fn);
    static auto* mat_inst_const_cls = find_class(L"MaterialInstanceConstant"_fn);
    UObject* alt_material = material->get<UFunction, BoundFunction>(fn_get_material).call<ZObjectProperty>();

    const GObjects& all_objects = gobjects();
    for (size_t i = 0; i < all_objects.size(); ++i) {
        UObject* obj = all_objects.obj_at(i);
        if (obj == nullptr || !obj->is_instance(mat_inst_const_cls)) {
            continue;
        }
        UObject* mat = obj->get<UFunction, BoundFunction>(fn_get_material).call<ZObjectProperty>();
        if (mat == material || mat == alt_material) {
            return mat;
        }
    }
    return nullptr;
}

}  // namespace

}  // namespace world_exporter
