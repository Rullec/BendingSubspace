from physical_prop_helper import *
import pickle as pkl


def get_warp_weft_bias_value(bending_lst):
    assert 8 == len(bending_lst)
    # 0, 22.5, 45, 67.5, 90, 112.5, 135, 157.5 
    warp = bending_lst[0]
    bias = bending_lst[2]
    weft = bending_lst[4]
    return warp, weft, bias


from iterative_solver import *


def calculate_sim_param(density, warp_length, weft_length, bias_length):
    '''
    @density: [g.m^{-2}]
    @warp_length: [m]
    @weft_length: [m]
    @bias_length: [m]
    '''
    factor_matrix = get_factor_matrix()

    if_use_nonlinear = use_nonlinear(weft_length, warp_length)

    obs_warp_length = get_bending_observed_stiffness(density, warp_length)
    obs_weft_length = get_bending_observed_stiffness(density, weft_length)
    obs_bias_length = get_bending_observed_stiffness(density, bias_length)
    obs_bias_length = bias_clamp(obs_weft_length, obs_warp_length,
                                 obs_bias_length)

    rhs = np.array([obs_weft_length, obs_warp_length, obs_bias_length])

    Strhs = np.dot(factor_matrix, rhs)
    b = Strhs

    mat = get_GS_mat(factor_matrix)
    if if_use_nonlinear:
        bending_stiff = bending_nonlinear_gauss_seidel(mat, b)
    else:
        bending_stiff = bending_linear_gauss_seidel(mat, b)
    return bending_stiff


if __name__ == "__main__":
    bending_stiffness_pkl = r"..\Step1FromBezierToBendingStiffness\output\bending_stiffness_data.pkl"

    output_pkl = r"output\bending_sim_param.pkl"

    with open(bending_stiffness_pkl, 'rb') as f:
        cont = pkl.load(f)
    name_lst = cont["name_lst"]
    idx_lst = cont["idx_lst"]
    density_lst = cont["density_lst"]
    linear_density_lst = cont["linear_density_lst"]
    front_bending_length_lst = cont["front_bending_length_lst"]
    back_bending_length_lst = cont["back_bending_length_lst"]
    angles = cont["angles"]
    unit = cont["unit"]

    num_of_data = len(cont['name_lst'])
    front_sim_param_lst = []
    back_sim_param_lst = []
    for _idx in range(num_of_data):
        fabric_id = idx_lst[_idx]
        name = name_lst[_idx]
        density = density_lst[_idx]  # [kg.m^{-2}]
        front_warp_length, front_weft_length, front_bias_length = get_warp_weft_bias_value(
            front_bending_length_lst[_idx])  # [m]
        back_warp_length, back_weft_length, back_bias_length = get_warp_weft_bias_value(
            back_bending_length_lst[_idx])  # [m]

        # get the simulation internal param
        front_warp_sim, front_weft_sim, front_bias_sim = calculate_sim_param(
            density * 1e3, front_warp_length, front_weft_length,
            front_bias_length)
        back_warp_sim, back_weft_sim, back_bias_sim = calculate_sim_param(
            density * 1e3, back_warp_length, back_weft_length,
            back_bias_length)

        front_warp_gui = convert_bending_simtogui(front_warp_sim)
        front_weft_gui = convert_bending_simtogui(front_weft_sim)
        front_bias_gui = convert_bending_simtogui(front_bias_sim)

        back_warp_gui = convert_bending_simtogui(back_warp_sim)
        back_weft_gui = convert_bending_simtogui(back_weft_sim)
        back_bias_gui = convert_bending_simtogui(back_bias_sim)
        # print("---------------")
        # print(fabric_id, name, f"density {density} kg.m^{-2}")
        # print(f"重量 {density * 0.22 * 0.03 * 1e3} g")
        # print("front length (mm)", f"warp(经) {back_warp_length * 1e3}",
        #       f"weft(纬) {back_weft_length * 1e3}",
        #       f"bias {back_bias_length * 1e3}")
        # print(
        #     f"front sim internal: warp(经) {front_warp_sim}, weft(纬) {front_weft_sim}, bias {front_bias_sim}"
        # )
        print(f"------{fabric_id}, {name}------")
        print("front gui", front_warp_gui, front_weft_gui, front_bias_gui)
        print("back gui", back_warp_gui, back_weft_gui, back_bias_gui)
        front_sim_param_lst.append({
            "warp": front_warp_gui,
            "weft": front_weft_gui,
            "bias": front_bias_gui
        })
        back_sim_param_lst.append({
            "warp": back_warp_gui,
            "weft": back_weft_gui,
            "bias": back_bias_gui
        })

        # print(fabric_id, name, "back gui", back_warp_gui, back_weft_gui,
        #       back_bias_gui)

    cont["front_sim_param_lst"] = front_sim_param_lst
    cont["back_sim_param_lst"] = back_sim_param_lst

    import os.path as osp
    import os
    output_dir = osp.dirname(output_pkl)
    if osp.exists(output_dir) is False:
        os.makedirs(output_dir)
    with open(output_pkl, 'wb') as f:
        pkl.dump(cont, f)
    print(f"write bending sim param pkl to {output_pkl}")