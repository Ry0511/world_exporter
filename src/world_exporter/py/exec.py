from typing import List, cast
from pathlib import Path
from unrealsdk.unreal import UObject, WrappedStruct, UStructProperty
from unrealsdk import find_all, find_class, find_enum, find_object

from world_exporter import export_world

dest = Path(__file__).parent / "exports"
export_world(dest)

texture_params = [
    "p_Diffuse",
    "p_Normal",
    "p_Emissive",
]

# Prop_Skybox.Materials.Mat_SmokePlume_03
# Prop_Rocks_03.Materials.Mat_Volcanic
# Prop_Rocks.Materials.Mat_RockBluff
# Prop_Water.Materials.Mat_Riverbed
# Prop_Freighter.Material.Mat_FreighterSkybox
# Common_Materials.Environment.Master_Water_opaque
# Prop_Skybox.Materials.Mat_VolcanusLow
# Prop_PandoraPark.Materials.Mat_Dome_Skybox
# Prop_Buildings.Materials.Mat_NHBuilding2
# Prop_Craters.Materials.Mati_WormsquidCrater
# Prop_Plants.Materials.Mat_InterludeDeadGrass

# mat = find_object("Material", "Common_Materials.Environment.Master_Water_opaque")
# print(mat.Normal)
# for expr in mat.Expressions:
#     if expr is not None:
#         print(f"{expr.ParameterName}: {expr}")

# for obj in find_all("MaterialInstance", False):
#     if obj.Class.Name == "MaterialInstanceConstant":
#         continue
#     print(obj)
#     for param in texture_params:
#         ok, tex = obj.GetTextureParameterValue(param, None)
#         print(f" - {param:>12} => {tex}")
#     print("-" * 60)
