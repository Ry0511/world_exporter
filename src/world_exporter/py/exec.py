import sys
from pathlib import Path
from unrealsdk import find_all

file_dir = str(Path(__file__).parent.absolute())

if file_dir not in sys.path:
    sys.path.append(file_dir)

i = 0

for world in find_all("StaticMesh", False):
    if int(world.ObjectFlags) & (0x400 | 0x200):
        continue
    print(world._path_name())
    if i == 5:
        import world_exporter

        dest = str(Path.cwd() / "test_file.txt")
        world_exporter.export_static_mesh(dest, world._path_name())
    i += 1
