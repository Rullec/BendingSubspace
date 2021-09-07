from load_bending_bezier_data import get_fabric_subdirs, fetch_fabric_data, load_bezier_datamat, load_captured_image
import os

from solve_bending_equation import shoot_solve_normalzed
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
    # 1. read the bezier curve parameters (SI)
    A, B, C, D, unit_cm, img_filename, projective2d = load_bezier_datamat(
        bezier_data_path)
    img, origin_mode = load_captured_image(image_data_path, projective2d)
    raw_A = A
    A, B, C, D = move_the_bezier_origin_and_mirror(A, B, C, D)

    A, B, C, D = normalize_from_pixel_to_meter(unit_cm, A, B, C, D)  # [m]

    linera_rho = get_linear_density_for_specimen(root_dir)  # [kg / m]
    rho_g = 9.8 * linera_rho  # m. s^{-2}. kg. m^{-1}
    bezier_curve = BezierCurve(A, B, C, D)
    x_lst, y_lst = bezier_curve.get_discretized_point_lst()
    M_lst = bezier_curve.get_Torque_lst(rho_g)
    K_lst = bezier_curve.get_curvatured_lst()
    G_lst = M_lst / K_lst

    y = np.array(M_lst)
    x = np.array(K_lst)
    if x.shape != y.shape:
        return
    x = x[y > 0]
    y = y[y > 0]

    k = np.dot(x, y) / np.dot(x, x)
    new_y = k * x
    # l = bezier_curve.get_total_arc_length()
    # print(f"begin to solve")
    theta_lst = bezier_curve.get_theta_lst()
    theta0 = theta_lst[0]

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
    t0 = (k - k_min) / (k_max - k_min) * 0.1 + 0.05
    t0 = min(t0, 0.3)

    stepsize = 1e-1
    if k < 1e-6:
        stepsize *= 0.1

    beta = -1 * new_rho_g * (L**3) / new_k
    view_x, view_y = shoot_solve_normalzed(beta, theta0, t0, stepsize)  # [cm]

    if view_x == None and view_y == None:
        print(f"[error] solve {bezier_data_path} failed, G={k}")
        return (root_dir, bezier_data_path, image_data_path, output_name)
    else:
        print(f"[log] solve {bezier_data_path} succ, G={k}")
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
    return None


def generate_param_lst(root_dir):

    idx = 0
    fabric_subdir_lst = get_fabric_subdirs(root_dir)
    param_lst = []
    output_dir = "output"
    if osp.exists(output_dir):
        import shutil
        shutil.rmtree(output_dir)
    os.makedirs(output_dir)

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


if __name__ == "__main__":
    root_dir = "D:\\Projects\\弯曲测量数据"
    param_lst = generate_param_lst(root_dir)

    # param_lst = param_lst[:20]

    # for i in param_lst:
    #     calculate_bending_stiffness_from_bezier(*i)
    from multiprocessing import Pool
    with Pool(8) as pool:
        ret = pool.starmap(calculate_bending_stiffness_from_bezier, param_lst)
    for i in ret:
        if i != None:
            print(f"{i} solve failed")