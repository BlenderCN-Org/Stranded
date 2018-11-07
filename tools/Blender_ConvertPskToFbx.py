#2.79b


import bpy
import os

def psk_to_fbx(in_path,out_path):
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete()
    bpy.ops.import_scene.psk(filepath=in_path)
    bpy.ops.export_scene.fbx(filepath=out_path)

def psa_to_fbx(in_path,out_path):
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete()
    bpy.ops.import_scene.psa(filepath=in_path)
    bpy.ops.export_scene.fbx(filepath=out_path)

def convert(psk_path, ext):
    (path, name) = os.path.split(psk_path)
    (in_name, in_ext) = os.path.splitext(name)
    out_path = os.path.join(path, in_name + ".fbx")

    if ext == ".psa":
        psa_to_fbx(psk_path, out_path)
    else:
        psk_to_fbx(psk_path, out_path)

    os.remove(psk_path)
    print(out_path)


def find_file(path):

    for file_name in os.listdir(path):
        file_path = os.path.join(path, file_name)

        if os.path.isfile(file_path):
            file_e = os.path.splitext(file_path)[1]
            if file_e == ".psk" or file_e == ".pskx" or file_e == ".psa":
                convert(file_path, file_e)

        if os.path.isdir(file_path):
            find_file(file_path)


if __name__ == '__main__':
    input_path = input("input directory:")
    find_file(input_path)





