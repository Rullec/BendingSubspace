from load_bending_bezier_data import get_fabric_subdirs, fetch_fabric_data, load_bezier_datamat, load_captured_image, get_delimiter
import os

from solve_bending_equation import shoot_solve_normalzed
from PIL import Image
import numpy as np
import os.path as osp
from bezier_curve import BezierCurve
import matplotlib.pyplot as plt
import pickle as pkl
# 1. load the bezier control point, the disretization points on bezier curve, the 1st derivative and the 2nd derivative

import sys

sys.path.append("D:\Projects\Subspace\Step2FromGBToSim")
from calculate_sim_param import calculate_sim_param, convert_bending_simtogui

specimen_width = 3  # cm
specimen_length = 22  # cm


def normalize_from_pixel_to_meter(unit_cm, *args):
    assert type(unit_cm) is float
    new_data = [i / unit_cm * 1e-2 for i in args]
    return new_data


def unnormalize_from_meter_to_pixel(unit_cm, *args):
    assert type(unit_cm) is float
    new_data = [i / 1e-2 * unit_cm for i in args]

    return new_data


def get_specimen_width_cm():
    global specimen_width
    return specimen_width


def get_specimen_length_cm():
    global specimen_length
    return specimen_length


def get_linear_density_for_specimen(fabric_dir, idx):
    specimen_width = get_specimen_width_cm()
    specimen_length = get_specimen_length_cm()
    delimiter = get_delimiter()

    # idx_str = fabric_dir.split(delimiter)[-1]
    # idx = int(idx_str)
    root_dir = delimiter.join(fabric_dir.split(delimiter)[:-1])
    rho_file = osp.join(root_dir, "linear_density_rec.pkl")
    if osp.exists(rho_file) is False:
        # record_file = osp.join(root_dir, "厚度和重量.csv")
        record_file = r"D:\Projects\弯曲测量数据\厚度和重量.csv"
        lines = []
        with open(record_file, 'r') as f:
            for line in f.readlines():
                cont = [
                    i.strip() for i in line.split(',') if len(i.strip()) != 0
                ]
                lines.append(cont)

        # for line in lines:
        #     print(line)

        linear_density_dict = {}
        for line in lines[1:]:
            id = int(line[0])
            weight = float(line[2])  # weight for specimen
            linear_rho = (weight * 1e-3) / (
                specimen_length * 1e-2 * specimen_width * 1e-2)  # [kg / m]
            linear_density_dict[id] = linear_rho
        linear_density_dict["comment"] = f"the width is {specimen_width}"
        with open(rho_file, 'wb') as f:
            pkl.dump(linear_density_dict, f)

    with open(rho_file, 'rb') as f:
        linear_rho = pkl.load(f)[idx]
    return linear_rho


def move_the_bezier_origin_and_mirror(A, B, C, D):
    B = B - A
    C = C - A
    D = D - A
    A = A - A

    B[0][0] *= -1
    C[0][0] *= -1
    D[0][0] *= -1
    return A, B, C, D


import time


def draw_bending_curve(bezier_curve, k, rho_g, unit_cm, img, raw_A,
                       output_name, origin_mode):
    x_lst, y_lst = bezier_curve.get_discretized_point_lst()
    # l = bezier_curve.get_total_arc_length()
    # print(f"begin to solve")
    theta_lst = bezier_curve.get_theta_lst()
    theta0 = theta_lst[0]
    # print(f"k = {k}, rho*G {rho_g}")
    # exit()
    # convert G from kg.s^{-2}.m^3 -> g.cm.s^{-2}.cm^2
    new_k = 1e9 * k
    # convert rho * g from kg.s^{-2} to g.s^{-2}
    new_rho_g = rho_g * 1e3
    # convert length from m to cm
    L = bezier_curve.total_arc_length * 1e2

    # call the subroutine
    # t0 = 0.1
    k_min = 1e-7
    k_max = 1e-5
    t0 = (k - k_min) / (k_max - k_min) * 0.1
    t0 = min(t0, 0.3)

    stepsize = 1e-1
    if k < 1e-6:
        stepsize *= 0.1
    if k < 3e-7:
        stepsize *= 0.1
    beta = -1 * new_rho_g * (L**3) / new_k
    print(f"new k {new_k}")
    print(f"new rho_g {new_rho_g}")
    print(f"L {L}")
    print(f"stepsize {stepsize}")
    print(f"beta {beta}")
    print(f"theta0 {theta0}")
    print(f"t0 {t0}")
    # exit()
    view_x, view_y = shoot_solve_normalzed(beta, theta0, t0, stepsize)  # [cm]

    # print(
    #     f"[log] solve {bezier_data_path} done, G={k:.3e}, rhog = {rho_g:.3f}, beta = {beta:.3f}"
    # )
    x_lst = unnormalize_from_meter_to_pixel(unit_cm, *x_lst)
    y_lst = unnormalize_from_meter_to_pixel(unit_cm, *y_lst)
    x_lst = [i + img.shape[1] - raw_A[0][0] for i in x_lst]
    y_lst = [i + raw_A[0][1] for i in y_lst]
    plt.plot(x_lst, y_lst, label="raw bending curve")

    view_x = np.array(view_x) * L
    view_y = np.array(view_y) * L

    # view x from cm to m
    view_x *= 1e-2
    view_y *= 1e-2
    view_x = unnormalize_from_meter_to_pixel(unit_cm, *view_x)
    view_y = unnormalize_from_meter_to_pixel(unit_cm, *view_y)
    diff_x = x_lst[0] - view_x[0]
    diff_y = y_lst[0] - view_y[0]
    view_x = [i + diff_x for i in view_x]
    view_y = [i + diff_y for i in view_y]
    plt.plot(view_x, view_y, label='calc from param')
    plt.legend()
    plt.imshow(img[:, ::-1, :], origin=origin_mode)
    plt.title(f"bending stiff = {k:.3e} [N.m^2]")

    if output_name is None:
        plt.show()
    else:
        plt.savefig(output_name)
        print(f"[log] save result figure to {output_name}")
    plt.cla()
    plt.clf()
    plt.close('all')


def calculate_arc_length_bezier(root_dir, bezier_data_path):
    bezier_data_path = osp.join(root_dir, bezier_data_path)
    A, B, C, D, unit_cm, img_filename, projective2d = load_bezier_datamat(
        bezier_data_path)
    A, B, C, D = normalize_from_pixel_to_meter(unit_cm, A, B, C, D)  # [m]
    bezier = BezierCurve(A, B, C, D, cutted_from_the_biggest_curvature=False)
    return bezier.get_total_arc_length()


def calculate_bending_stiffness_from_bezier(
        root_dir,
        bezier_data_path,
        image_data_path,
        fabric_idx,
        draw_resolved_curve=False,
        output_name=None,
        cutted_from_the_biggest_curvature=True):
    '''
        G: bending stiffness [N \cdot m^2]
        K: curvature [m^{-1}]
        M: bending torque [N \cdot m]

        we have: G = M / K
    '''
    bezier_data_path = osp.join(root_dir, bezier_data_path)
    image_data_path = osp.join(root_dir, image_data_path)

    # 1. read the bezier curve parameters (SI)
    A, B, C, D, unit_cm, img_filename, projective2d = load_bezier_datamat(
        bezier_data_path)
    # print(f"raw A {A} B {B} C {C} D {D} unitCM {unit_cm}")
    raw_A = A
    A, B, C, D = move_the_bezier_origin_and_mirror(A, B, C, D)

    A, B, C, D = normalize_from_pixel_to_meter(unit_cm, A, B, C, D)  # [m]

    linera_rho = get_linear_density_for_specimen(root_dir,
                                                 fabric_idx)  # [kg / m]
    rho_g = 9.8 * linera_rho  # m. s^{-2}. kg. m^{-1}
    # print(f"for data {bezier_data_path}")
    # print(f"new A {A} B {B} C {C} D {D} unitCM {unit_cm}")
    # exit()
    bezier_curve = BezierCurve(
        A,
        B,
        C,
        D,
        cutted_from_the_biggest_curvature=cutted_from_the_biggest_curvature,
    )

    # plt.plot(bezier_curve.x_lst, bezier_curve.y_lst)
    # plt.show()
    # exit()
    M_lst = bezier_curve.get_Torque_lst(rho_g)
    K_lst = bezier_curve.get_curvatured_lst()
    G_lst = M_lst / K_lst

    y = np.array(M_lst)
    x = np.array(K_lst)
    if x.shape != y.shape:
        min_shape = min(len(y), len(x))
        y = y[:min_shape]
        x = x[:min_shape]
    x = x[y > 0]
    y = y[y > 0]

    k = np.dot(x, y) / np.dot(x, x)
    if np.isnan(k):
        assert len(y) == 0
        print(f"all curvature is negative, set this value to 0")
        k = 0
    print(bezier_data_path, k)
    # exit()
    # -------------- done, the below code are used to resolve the diff eq-----------
    # if draw_resolved_curve == True:
    img, origin_mode = load_captured_image(image_data_path, projective2d)
    draw_bending_curve(bezier_curve, k, rho_g, unit_cm, img, raw_A,
                       output_name, origin_mode)

    # assemble the info matrix
    return k


def generate_param_lst(root_dir):

    idx = 0
    fabric_subdir_lst = get_fabric_subdirs(root_dir)
    param_lst = []
    output_dir = "output"
    if osp.exists(output_dir) is False:
        os.makedirs(output_dir)
        # import shutil
        # shutil.rmtree(output_dir)

    for fabric_subdir in fabric_subdir_lst:
        front_image_data, front_bezier_data, back_image_data, back_bezier_data = fetch_fabric_data(
            fabric_subdir)
        image_data = front_image_data + back_image_data
        bezier_data = front_bezier_data + back_bezier_data

        # print(f"image data {image_data}")
        # print(f"bezier data {bezier_data}")
        for img, bezier_data in zip(image_data, bezier_data):
            output_png = os.path.join(output_dir, f"{idx}.png")
            if osp.exists(output_png) == False:
                # ret = calculate_bending_stiffness_from_bezier(
                #     fabric_subdir, bezier_data, img, output_name=output_png)
                param_lst.append((fabric_subdir, bezier_data, img, output_png))
                # if ret != None:
                #     print(f"[warn] solve {ret} failed!")
            idx += 1
    return param_lst


def calculate_all_param():
    root_dir = "D:\\RealMeasureData\\BendingMeasureData"
    param_lst = generate_param_lst(root_dir)
    for param in param_lst:
        root_dir, bezier_data_path, image_data_path, output_name = param
        fabric_idx = int(root_dir.split("\\")[-1])
        calculate_bending_stiffness_from_bezier(
            root_dir,
            bezier_data_path,
            image_data_path,
            fabric_idx,
            output_name=output_name,
            cutted_from_the_biggest_curvature=True)
    exit()


g = 9.81


def calculate_specified_param():
    root_dir = 'D:\\Projects\\弯曲测量数据\\40'
    data_mat_lst = ['Front-0.mat', 'Front-45.mat', 'Front-90.mat']
    img_lst = ['DSC_1726.JPG', 'DSC_1730.JPG', 'DSC_1734.JPG']
    fabric_idx = 40
    # root_dir = 'D:\\Projects\\弯曲测量数据\\46'
    # data_mat_lst = ['Front-0.mat',
    #     'Front-45.mat',
    #     'Front-90.mat']
    # img_lst = ['DSC_2242.JPG',
    #         'DSC_2247.JPG',
    #         'DSC_2251.JPG']
    # fabric_idx = 46
    # root_dir = 'D:\\Projects\\弯曲测量数据\\46'
    # data_mat_lst = ['Front-0.mat',
    #     'Front-45.mat',
    #     'Front-90.mat']
    # img_lst = ['DSC_2242.JPG',
    #         'DSC_2247.JPG',
    #         'DSC_2251.JPG']
    # fabric_idx = 46
    # root_dir = 'D:\\Projects\\弯曲测量数据\\133'
    # data_mat_lst = ['Front-0.mat',
    #     'Front-45.mat',
    #     'Front-90.mat']
    # img_lst = ['DSC_0460.JPG',
    #         'DSC_0464.JPG',
    #         'DSC_0468.JPG']
    # fabric_idx = 133

    assert len(img_lst) == len(data_mat_lst)
    bending_stiff_lst = []
    for i in range(len(img_lst)):
        param = (root_dir, data_mat_lst[i], img_lst[i])
        k = calculate_bending_stiffness_from_bezier(
            *param,
            fabric_idx=fabric_idx,
            cutted_from_the_biggest_curvature=True)
        bending_stiff_lst.append(k)
    bending_stiff_lst = np.array(bending_stiff_lst)
    linear_density = get_linear_density_for_specimen(root_dir, fabric_idx)
    density = 1 / (1e-2 * get_specimen_width_cm()) * linear_density
    bending_length_lst = 2 * np.power(bending_stiff_lst /
                                      (linear_density * g), 1 / 3)
    print(f"k {bending_stiff_lst}")
    print(f"length {bending_length_lst}")

    warp_sim, weft_sim, bias_sim = calculate_sim_param(
        density * 1e3,
        bending_length_lst[0],
        bending_length_lst[2],
        bending_length_lst[1],
    )
    warp_gui = convert_bending_simtogui(warp_sim)
    weft_gui = convert_bending_simtogui(weft_sim)
    bias_gui = convert_bending_simtogui(bias_sim)

    print(f"sim param {warp_gui} {weft_gui} {bias_gui}")


if __name__ == "__main__":

    # BezierCurve(A, B, C, D, n = 5, cutted_from_the_biggest_curvature=False)
    calculate_all_param()
    # calculate_specified_param()