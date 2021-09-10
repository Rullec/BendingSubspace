import numpy as np
import pickle as pkl
import os.path as osp
import sys

sys.path.append(
    osp.join(osp.dirname(__file__), "../Step1FromBezierToBendingStiffness"))
sys.path.append(osp.join(osp.dirname(__file__), "../Step2FromGBToSim"))
sys.path.append(osp.join(osp.dirname(__file__), "../Step3Subspace"))
from calculate_bending_stiffness_from_bezier import get_specimen_width_cm, get_specimen_length_cm
from calculate_sim_param import calculate_sim_param
from physical_prop_helper import convert_bending_simtogui
from draw_3d import Draw3D


def load_extended_data(csv_path):

    with open(csv_path, 'r') as f:
        conts = f.readlines()[1:]
    # 1. get the item name, calculate the intersted
    govern_line = conts[0].strip()
    govern_line = govern_line.split(",")
    conts = conts[1:]

    def find_location(key, govern_line):
        tar_idx = None
        for _idx, i in enumerate(govern_line):
            if i.find(key) != -1:
                assert tar_idx is None
                tar_idx = _idx
        assert tar_idx is not None, f"key {key} not found in {govern_line}"
        return tar_idx

    id_key, name_key, thickness_key, kezhong_key, weft_key, warp_key, bias_key = "序号", "面料名称", "厚度", "克重/每张", "纬/mm", "经/mm", "斜/mm"

    id_idx = find_location(id_key, govern_line)
    name_idx = find_location(name_key, govern_line)
    thickness_idx = find_location(thickness_key, govern_line)
    kezhong_idx = find_location(kezhong_key, govern_line)
    weft_idx = find_location(weft_key, govern_line)
    warp_idx = find_location(warp_key, govern_line)
    bias_idx = find_location(bias_key, govern_line)

    # 2. load the value
    name_lst = []
    idx_lst = []
    density_lst = []
    warp_bending_length_lst = []
    weft_bending_length_lst = []
    bias_bending_length_lst = []
    for line in conts:
        line = line.strip().split(",")
        fabric_id = int(line[id_idx])
        name = line[name_idx]
        # thickness = float(line[thickness_idx]) * 1e-3  # mm to m
        kezhong = float(line[kezhong_idx]) * 1e-3  # g to kg
        weft_length = float(line[weft_idx]) * 1e-3  # mm to m
        warp_length = float(line[warp_idx]) * 1e-3  # mm to m
        bias_length = float(line[bias_idx]) * 1e-3  # mm to m

        # kezhong to density [kg/m^{2}]
        density = kezhong / (get_specimen_length_cm() * 1e-2 *
                             get_specimen_width_cm() * 1e-2)
        # linear density [kg/m]
        # linear_density = density * get_specimen_width_cm() * 1e-2

        # adding
        name_lst.append(name)
        idx_lst.append(fabric_id)
        density_lst.append(density)
        warp_bending_length_lst.append(warp_length)
        weft_bending_length_lst.append(weft_length)
        bias_bending_length_lst.append(bias_length)
        # print(fabric_id, name, thickness, kezhong, weft, warp, bias)
        unit = "N.m^2"
    num_of_angles = 8
    angles = [i * 22.5 for i in range(num_of_angles)]

    return name_lst, idx_lst, density_lst, warp_bending_length_lst, weft_bending_length_lst, bias_bending_length_lst,


def load_single_csv(csv_path):
    name_lst, idx_lst, density_lst, warp_length_lst, weft_length_lst, bias_length_lst = load_extended_data(
        csv_path)
    num_of_items = len(name_lst)
    sim_param_lst = []
    for i in range(num_of_items):
        warp_sim, weft_sim, bias_sim = calculate_sim_param(
            density_lst[i] * 1e3, warp_length_lst[i], weft_length_lst[i],
            bias_length_lst[i])

        warp_gui = convert_bending_simtogui(warp_sim)
        weft_gui = convert_bending_simtogui(weft_sim)
        bias_gui = convert_bending_simtogui(bias_sim)
        sim_param_lst.append({
            "warp": warp_gui,
            "weft": weft_gui,
            "bias": bias_gui
        })
        # print(name_lst[i], warp_length_lst[i], weft_length_lst[i],
        #       bias_length_lst[i], warp_gui, weft_gui, bias_gui)
        # print(name_lst[i], warp_gui, weft_gui, bias_gui)
    return name_lst, idx_lst, sim_param_lst


if __name__ == "__main__":
    # 1. load the csv, get the bending length in SI
    data_root = "D:\Projects\第二批面料弯曲数据(精简)-20210910"
    output_pkl = r"../Step2FromGBToSim\output\bending_sim_param_extended.pkl"
    csv_paths = ["安踏弯曲数据(截面法).csv", "Shein弯曲数据(截面法).csv", "内部弯曲数据(截面法).csv"]
    name_lst = []
    idx_lst = []
    sim_param_lst = []
    drawer = Draw3D()
    colors = ["red", "blue", "green"]
    for _idx, csv_path in enumerate(csv_paths):
        path = osp.join(data_root, csv_path)
        __name_lst, __idx_lst, __sim_param_lst = load_single_csv(path)
        name_lst += __name_lst
        idx_lst += __idx_lst
        sim_param_lst += __sim_param_lst
        drawer.add_points([i["warp"] for i in __sim_param_lst],
                          [i["weft"] for i in __sim_param_lst],
                          [i["bias"] for i in __sim_param_lst],
                          label=csv_path,
                          color=colors[_idx],
                          alpha=0.5)
    # print(len(name_lst))
    # print(len(idx_lst))
    # print()
    drawer.draw()
    cont = {}
    cont["name_lst"] = name_lst
    cont["idx_lst"] = idx_lst
    cont["front_sim_param_lst"] = sim_param_lst
    cont["back_sim_param_lst"] = []

    with open(output_pkl, 'wb') as f:
        pkl.dump(cont, f)
        print(f"dump {len(sim_param_lst)} data to {output_pkl}")

    # 2. check the value and do visualization
    # drawer = Draw3D()
    # drawer.add_points(warp_bending_length_lst, weft_bending_length_lst, bias_bending_length_lst)
    # drawer.draw()

    # 4. calculate the sim parameter, combine with the old dataset
    # 5. output the new dataset
    # 6. training with the new dataset
    # 7. sampling from this dataset, do visualization