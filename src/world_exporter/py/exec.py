import sys
from pathlib import Path
from unrealsdk.unreal import UObject, WrappedStruct
from unrealsdk import find_all, find_class

file_dir = Path(__file__).parent.absolute()
export_dir = file_dir / "exports"

if str(file_dir) not in sys.path:
    sys.path.append(str(file_dir))

# for obj in find_all("StaticMeshComponent", False):
#
#     node = obj.Outer
#     while node:
#         if node.Name == "TheWorld":
#             break
#         node = node.Outer
#
#     if node:
#         print(obj)
#         print(obj.GetPosition())
#         obj.SetHidden(False)

from world_exporter import export_static_meshes
export_static_meshes(export_dir / "full_export.obj")

#
# i = 0
# for mesh in find_all("StaticMesh", False):
#     if int(mesh.ObjectFlags) & (0x400 | 0x200):
#         continue
#
#     print(f'{i:>3} {mesh.Name}')
#     if i == 10:
#         import world_exporter
#
#         print(mesh._path_name())
#         dest = file_dir / "exports" / (str(mesh.Name).lower() + ".obj")
#         dest.parent.mkdir(parents=True, exist_ok=True)
#         world_exporter.export_static_mesh(str(dest), mesh._path_name())
#     i += 1
