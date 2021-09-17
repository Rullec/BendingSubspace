import os
import numpy as np
import os.path as osp
import sys

sys.path.append("../Step1FromBezierToBendingStiffness")
from calculate_bending_stiffness_from_bezier_for_1d_evaluation import *
from calculate_bending_stiffness_from_bezier import get_linear_density_for_specimen, get_specimen_width_cm
from calculate_sim_param import calculate_sim_param, convert_bending_simtogui

g = 9.81


def calculate_bending_length(root_dir, fabric_idx):

    fabric_idx, data_dir, bezier_lst, img_lst = get_data(root_dir, fabric_idx)

    bengding_stiffness_lst = []
    for i in range(len(bezier_lst)):
        k = calculate_bending_stiffness_from_bezier(
            data_dir,
            osp.basename(bezier_lst[i]),
            osp.basename(img_lst[i]),
            fabric_idx=fabric_idx,
            cutted_from_the_biggest_curvature=False)
        bengding_stiffness_lst.append(k)
    bengding_stiffness_lst = np.array(bengding_stiffness_lst)

    linear_density = get_linear_density_for_specimen(root_dir, fabric_idx)
    # print(linear_density)
    density = 1 / (1e-2 * get_specimen_width_cm()) * linear_density

    bending_length = 2 * np.power(
        bengding_stiffness_lst / (linear_density * g), 1 / 3)
    print(f"k {bengding_stiffness_lst}")
    print(f"length {bending_length}")
    # warp, bias, weft
    warp_sim, weft_sim, bias_sim = calculate_sim_param(
        density * 1e3,
        bending_length[0],
        bending_length[2],
        bending_length[1],
    )
    warp_gui = convert_bending_simtogui(warp_sim)
    weft_gui = convert_bending_simtogui(weft_sim)
    bias_gui = convert_bending_simtogui(bias_sim)
    print(f"sim param {warp_gui} {weft_gui} {bias_gui}")


if __name__ == "__main__":
    root_dir = r"C:\Users\xudong\Desktop\一维评估水平实拍\40弹力贡缎"
    # root_dir = r"C:\Users\xudong\Desktop\一维评估水平实拍\46丝光卡其"
    # root_dir = r"C:\Users\xudong\Desktop\一维评估水平实拍\133摇粒绒"
    calculate_bending_length(root_dir, 40)