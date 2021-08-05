from load_bending_bezier_data import get_fabric_subdirs, fetch_fabric_data, load_bezier_datamat, load_captured_image
from solve_bending_equation import solve
from PIL import Image
import numpy as np
import os.path as osp
from bezier_curve import BezierCurve
import matplotlib.pyplot as plt
import pickle as pkl
# 1. load the bezier control point, the disretization points on bezier curve, the 1st derivative and the 2nd derivative


def normalize_from_pixel_to_meter(unit_cm, *args):
    assert type(unit_cm) is float
    new_data = [i / unit_cm * 1e-2 for i in args]
    return new_data


def unnormalize_from_meter_to_pixel(unit_cm, *args):
    assert type(unit_cm) is float
    new_data = [i / 1e-2 * unit_cm for i in args]

    return new_data


def get_linear_density_for_specimen(fabric_dir):
    idx = int(fabric_dir.split("\\")[-1])
    root_dir = '\\'.join(fabric_dir.split("\\")[:-1])
    rho_file = osp.join(root_dir, "linear_density_rec.pkl")
    if osp.exists(rho_file) is False:
        record_file = osp.join(root_dir, "厚度和重量.csv")
        lines = []
        with open(record_file, 'r') as f:
            for line in f.readlines():
                cont = [
                    i.strip() for i in line.split(',') if len(i.strip()) != 0
                ]
                lines.append(cont)

        # for line in lines:
        #     print(line)
        specimen_width = 3  # cm
        specimen_length = 22  # cm
        linear_density_dict = {}
        for line in lines[1:]:
            id = int(line[0])
            weight = float(line[2])  # weight for specimen
            linear_rho = (weight * 1e-3) / (specimen_length * 1e-2)  # [kg / m]
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


def calculate_bending_stiffness_from_bezier(root_dir,
                                            bezier_data_path,
                                            image_data_path,
                                            output_name=None):
    '''
        G: bending stiffness [N \cdot m^2]
        K: curvature [m^{-1}]
        M: bending torque [N \cdot m]

        we have: G = M / K
    '''
    bezier_data_path = osp.join(root_dir, bezier_data_path)
    image_data_path = osp.join(root_dir, image_data_path)
    # 1. calculate the torque M(x)
    ## 1.1 read the bezier point
    A, B, C, D, unit_cm, img_filename, projective2d = load_bezier_datamat(
        bezier_data_path)
    img, origin_mode = load_captured_image(image_data_path, projective2d)
    raw_A = A
    A, B, C, D = move_the_bezier_origin_and_mirror(A, B, C, D)

    A, B, C, D = normalize_from_pixel_to_meter(unit_cm, A, B, C, D)  # [m]

    # A *= 1e2  # cm
    # B *= 1e2  # cm
    # C *= 1e2  # cm
    # D *= 1e2  # cm

    # linera_rho = get_linear_density_for_specimen(root_dir) * 10 # [g / cm]
    linera_rho = get_linear_density_for_specimen(root_dir)  # [kg / m]
    # print(f"linera_rho {linera_rho} kg / m")
    # exit()
    rho_g = 9.8 * linera_rho  # m. s^{-2}. kg. m^{-1}
    bezier_curve = BezierCurve(A, B, C, D)
    x_lst, y_lst = bezier_curve.get_discretized_point_lst()
    # K_lst = bezier_curve.get_curvatured_lst()
    M_lst = bezier_curve.get_Torque_lst(rho_g)
    # print(f"M = {M_lst}")
    # exit()
    K_lst = bezier_curve.get_curvatured_lst()
    dKdx_lst = bezier_curve.get_dKdx()
    dMdx_lst = bezier_curve.get_dTorquedx(rho_g)
    # dMdx_lst_new = bezier_curve.get_dTorquedx_new(rho_g)
    # diff = (dMdx_lst_new - dMdx_lst) / dMdx_lst * 100
    # print(diff)
    # print(dMdx_lst_new)
    # exit()
    # G_lst_dM_divc_dK = dMdx_lst / dKdx_lst
    G_lst = M_lst / K_lst

    # plt.subplot(1, 3, 1)
    # plt.plot(K_lst)
    # plt.title("curvature")
    # plt.subplot(1, 3, 2)
    # plt.plot(M_lst)
    # plt.title("torque")
    # plt.subplot(1, 3, 1)
    # plt.scatter(K_lst, M_lst)
    y = np.array(M_lst)
    x = np.array(K_lst)
    if x.shape != y.shape:
        return
    x = x[y > 0]
    y = y[y > 0]

    k = np.dot(x, y) / np.dot(x, x)
    new_y = k * x

    print(f"approx G = {k}")
    # l = bezier_curve.get_total_arc_length()
    print(f"begin to solve")
    theta_lst = bezier_curve.get_theta_lst()
    theta0 = theta_lst[0]
    theta_minus1 = 2 * theta_lst[0] - theta_lst[1]
    # convert G from kg.s^{-2}.m^3 -> g.cm.s^{-2}.cm^2
    new_k = 1e9 * k
    # convert rho * g from kg.s^{-2} to g.s^{-2}
    new_rho_g = rho_g * 1e3
    # convert length from m to cm
    L = bezier_curve.total_arc_length * 1e2

    # print(f"G = {new_k}")
    # print(f"rhog = {new_rho_g}")
    # print(f"L = {L}")
    # print(f"theta0 = {theta0}, theta1 = {theta_minus1}")
    # exit()
    view_x, view_y = solve(new_k,
                           new_rho_g,
                           L,
                           theta0,
                           theta_minus1,
                           samples=1500)  # [cm]

    # print(f"end to solve")
    # exit()
    plt.title("M-K curve")
    plt.subplot(1, 3, 1)
    plt.scatter(x, new_y, color='red', label = "estimated")
    plt.plot(K_lst, M_lst, label = "real curve")
    plt.legend()

    plt.title("dM-dK curve")
    plt.subplot(1, 3, 2)
    plt.scatter(range(0, len(G_lst)), G_lst)
    plt.title("bending stiff")
    plt.suptitle('/'.join(bezier_data_path.split("\\")[-2:]))

    plt.subplot(1, 3, 3)
    # plt.subplot(1, 1, 1)
    x_lst = unnormalize_from_meter_to_pixel(unit_cm, *x_lst)
    y_lst = unnormalize_from_meter_to_pixel(unit_cm, *y_lst)
    x_lst = [i + img.shape[1] - raw_A[0][0] for i in x_lst]
    y_lst = [i + raw_A[0][1] for i in y_lst]
    plt.plot(x_lst, y_lst, label = "raw bending curve")

    view_x = np.array(view_x)
    view_y = np.array(view_y)
    solved_arc = np.sum( np.sqrt( np.diff(view_x) ** 2 + np.diff(view_y) ** 2))
    print(f"solved arc length {solved_arc} cm")
    # view x from cm to m
    view_x *= 1e-1
    view_y *= 1e-1
    view_x = unnormalize_from_meter_to_pixel(unit_cm, *view_x)
    view_y = unnormalize_from_meter_to_pixel(unit_cm, *view_y)
    diff_x = x_lst[0] - view_x[0]
    diff_y = y_lst[0] - view_y[0]
    view_x = [i + diff_x for i in view_x]
    view_y = [i + diff_y for i in view_y]
    plt.plot(view_x, view_y, label = 'calc from param')
    plt.legend()
    # y_lst += raw_A[0][1]
    plt.imshow(img[:, ::-1, :], origin=origin_mode)
    # plt.xlim(0, 1500)
    # plt.ylim(-1050, 450)
    # plt.gca().set_aspect('equal', adjustable='box')
    plt.title(f"bending stiff = {k:.3e} [N.m^2]")
    plt.show()
    return 
    # if output_name is None:
    # # else:
    #     plt.savefig(output_name)
    #     plt.cla()
    #     plt.clf()
    #     plt.close('all')
    # # exit()
    # return
    # exit()

    dKdx = bezier_curve.get_dKdx()
    dMdx = bezier_curve.get_dTorquedx(rho_g)
    bending_stiff = dMdx / dKdx
    print(f"bending_stiff {bending_stiff}")
    plt.plot(x_lst, y_lst)

    plt.xlim(-0.1, 0.1)
    plt.ylim(-0.1, 0.1)
    plt.gca().set_aspect('equal', adjustable='box')
    plt.show()
    plt.plot(bending_stiff)
    plt.show()
    exit()
    x_lst, y_lst = bezier_curve.get_discretized_point_lst()

    # x_lst = unnormalize_from_meter_to_pixel(unit_cm, *x_lst)
    # y_lst = unnormalize_from_meter_to_pixel(unit_cm, *y_lst)

    # image_data = img_warp(Image.open(osp.join(root_dir, image_data_path)),
    #                       projective2d)
    # image_data, imshow_origin = load_captured_image(
    #     osp.join(root_dir, image_data_path), projective2d)
    # plt.imshow(image_data, origin=imshow_origin)

    # print(type(x_lst), x_lst)
    # print(type(y_lst), y_lst)
    # exit()
    # plt.plot(x_lst, y_lst, color= 'red')
    # plt.show()
    # 2. calculate the curvature K(x)

    print(f"root dir {root_dir}")
    print(f"bezier data {bezier_data_path}")
    print(f"image data {image_data_path}")

    # exit(1)
    return None


# 2. calculate the curvature, do numiercal diff
# 3. calculate the bending stiffness

if __name__ == "__main__":
    root_dir = "D:\\Projects\\弯曲测量数据"
    idx = 0
    fabric_subdir_lst = get_fabric_subdirs(root_dir)
    for fabric_subdir in fabric_subdir_lst:
        front_image_data, front_bezier_data, back_image_data, back_bezier_data = fetch_fabric_data(
            fabric_subdir)
        image_data = front_image_data + back_image_data
        bezier_data = front_bezier_data + back_bezier_data

        # print(f"image data {image_data}")
        # print(f"bezier data {bezier_data}")
        for img, bezier_data in zip(image_data, bezier_data):
            output_png = f"{idx}.png"
            if osp.exists(output_png) == False:
                calculate_bending_stiffness_from_bezier(fabric_subdir,
                                                        bezier_data,
                                                        img,
                                                        output_name=output_png)
            idx += 1
