import os
import os.path as osp
from glob import glob
from load_bending_bezier_data import load_bezier_datamat, load_captured_image, get_delimiter
from matplotlib import pyplot as plt
from calculate_bending_stiffness_from_bezier import calculate_bending_stiffness_from_bezier, get_linear_density_for_specimen, calculate_arc_length_bezier
import re


def get_data(data_dir, fabric_idx):

    dir_name = data_dir.split("\\")[-1]
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
if __name__ == "__main__":
    root_dirs = [
        r"C:\Users\xudong\Desktop\一维评估水平实拍\40弹力贡缎",
        r"C:\Users\xudong\Desktop\一维评估水平实拍\46丝光卡其",
        r"C:\Users\xudong\Desktop\一维评估水平实拍\133摇粒绒"
    ]
    for root_dir in root_dirs:
        # root_dir = r"C:\Users\xudong\Desktop\一维评估水平实拍\40弹力贡缎"
        # root_dir = r"C:\Users\xudong\Desktop\一维评估水平实拍\46丝光卡其"
        # root_dir = r"C:\Users\xudong\Desktop\一维评估水平实拍\133摇粒绒"
        fabric_idx = int(re.findall(r'\d+', root_dir)[0])
        fabric_idx, data_dir, bezier_lst, img_lst = get_data(
            root_dir, fabric_idx)
        assert len(bezier_lst) == len(img_lst)
        k_lst = []
        arc_length_lst = []
        name = osp.basename(root_dir)
        for i in range(len(bezier_lst)):
            k = calculate_bending_stiffness_from_bezier(
                data_dir,
                osp.basename(bezier_lst[i]),
                osp.basename(img_lst[i]),
                fabric_idx=fabric_idx,
                cutted_from_the_biggest_curvature=False)
            arc_length = calculate_arc_length_bezier(
                data_dir, osp.basename(bezier_lst[i]))
            k_lst.append(k)
            arc_length_lst.append(arc_length)
        k_lst = np.array(k_lst)
        arc_length_lst = np.array(arc_length_lst)
        np.set_printoptions(
            formatter={'float': lambda x: "{0:0.3e}".format(x)})
        print(
            name,
            f"(0°, 45°, 90°): stiffness {k_lst} arc_length {arc_length_lst}[m]"
        )
    # # 1. get root dir
    # k = calculate_bending_stiffness_from_bezier(
    #     data_dir,
    #     osp.basename(front0_bezier),
    #     osp.basename(front0_img),
    #     fabric_idx=fabric_idx,
    #     cutted_from_the_biggest_curvature=False)
    # print(osp.basename(front0_bezier), k)
    # k = calculate_bending_stiffness_from_bezier(
    #     data_dir,
    #     osp.basename(front45_bezier),
    #     osp.basename(front45_img),
    #     fabric_idx=fabric_idx,
    #     cutted_from_the_biggest_curvature=False)
    # print(osp.basename(front45_bezier), k)
    # k = calculate_bending_stiffness_from_bezier(
    #     data_dir,
    #     osp.basename(front90_bezier),
    #     osp.basename(front90_img),
    #     fabric_idx=fabric_idx,
    #     cutted_from_the_biggest_curvature=False)
    # print(osp.basename(front90_bezier), k)
    # # img, origin_mode = load_captured_image(front0_img, projective2d)
