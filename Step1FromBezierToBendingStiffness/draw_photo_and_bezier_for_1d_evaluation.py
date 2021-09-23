import os
import json
import os.path as osp
from glob import glob
from load_bending_bezier_data import load_bezier_datamat, load_captured_image, get_delimiter, draw_bezier
from matplotlib import pyplot as plt
from calculate_bending_stiffness_from_bezier import calculate_bending_stiffness_from_bezier, get_linear_density_for_specimen, calculate_arc_length_bezier, normalize_from_pixel_to_meter, move_the_bezier_origin_and_mirror
import re


def get_data(data_dir, fabric_idx):

    dir_name = osp.basename(data_dir)
    assert int(re.findall(r'\d+', dir_name)[0]) == fabric_idx
    # files = os.listdir(data_dir)
    front0_img = [
        i for i in glob(f"{data_dir}\\*0度.jpg") if i.find("90") == -1
    ][0]
    front45_img = glob(f"{data_dir}\\*45度.jpg")[0]
    front90_img = glob(f"{data_dir}\\*90度.jpg")[0]

    front0_bezier = f"{data_dir}\\Front-0.mat"
    front45_bezier = f"{data_dir}\\Front-45.mat"
    front90_bezier = f"{data_dir}\\Front-90.mat"

    assert osp.exists(front0_bezier) == True
    assert osp.exists(front45_bezier) == True
    assert osp.exists(front90_bezier) == True
    # A, B, C, D, unit_cm, img_filename, projective2d = load_bezier_datamat(
    #     front0_bezier)
    bezier_lst = [
        osp.basename(front0_bezier),
        osp.basename(front45_bezier),
        osp.basename(front90_bezier),
    ]
    img_lst = [
        osp.basename(front0_img),
        osp.basename(front45_img),
        osp.basename(front90_img),
    ]
    return fabric_idx, data_dir, bezier_lst, img_lst


import numpy as np

g = 9.81


def transform_bezier_pts(A, B, C, D):
    B = B - A
    C = C - A
    D = D - A
    A = A - A
    A = list(A[0])
    B = list(B[0])
    C = list(C[0])
    D = list(D[0])
    return A, B, C, D


def display_evaluation_data(root_dir, bezier_lst, img_lst):
    # draw_bezier(root_dir, img_lst, bezier_lst, img_lst, bezier_lst)
    assert len(bezier_lst) == len(img_lst)
    '''
    {
        "angle" : angle,
        "img_path": path,
        "bezier_points": [A, B, C, D]
    }
    '''
    data_lst = []
    for i in range(len(bezier_lst)):
        angle = int(re.findall(r'\d+', bezier_lst[i])[0])
        bezier = osp.join(root_dir, bezier_lst[i])
        img_path = osp.join(root_dir, img_lst[i])
        A, B, C, D, unit_cm, given_img_path, proj2d = load_bezier_datamat(
            bezier)
        # print("old", A, B, C, D, unit_cm)
        A, B, C, D = normalize_from_pixel_to_meter(unit_cm, A, B, C, D)
        A, B, C, D = transform_bezier_pts(A, B, C, D)
        value = {
            "angle": int(angle),
            "img_path": img_path,
            "bezier_path": bezier,
            "bezier_pts": [A, B, C, D]
        }
        data_lst.append(value)
        # print("angle", angle, ":", A, B, C, D)
        # print(f"img path {img_path} bezier path {bezier}")
    return data_lst


if __name__ == "__main__":
    root_dirs = [
        r"D:\Projects\0914一维评估水平实拍\40弹力贡缎", r"D:\Projects\0914一维评估水平实拍\46丝光卡其",
        r"D:\Projects\0914一维评估水平实拍\133摇粒绒"
    ]
    output_dir = "bezier_record_dir"
    import shutil
    if os.path.exists(output_dir) == True:
        shutil.rmtree(output_dir)
    os.makedirs(output_dir)
    for root_dir in root_dirs:
        # root_dir = r"C:\Users\xudong\Desktop\一维评估水平实拍\40弹力贡缎"
        # root_dir = r"C:\Users\xudong\Desktop\一维评估水平实拍\46丝光卡其"
        # root_dir = r"C:\Users\xudong\Desktop\一维评估水平实拍\133摇粒绒"
        assert osp.exists(root_dir)
        root_base = osp.basename(root_dir)
        fabric_idx = int(re.findall(r'\d+', root_base)[0])
        fabric_idx, data_dir, bezier_lst, img_lst = get_data(
            root_dir, fabric_idx)
        assert len(bezier_lst) == len(img_lst)
        k_lst = []
        arc_length_lst = []
        name = osp.basename(root_dir)

        data = display_evaluation_data(root_dir, bezier_lst, img_lst)
        output_name = f"{output_dir}/{root_base}.json"
        json.dump(data, open(output_name, 'w'), ensure_ascii=False, indent = 4)
        print(f"output to {output_name}")