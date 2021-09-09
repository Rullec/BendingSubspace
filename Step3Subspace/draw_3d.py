import numpy as np
from tqdm import tqdm
import os.path as osp
import os
from matplotlib import pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import pickle as pkl


def get_data():
    pkl_path = r"..\Step2FromGBToSim\output\bending_sim_param.pkl"
    with open(pkl_path, 'rb') as f:
        cont = pkl.load(f)
    name_lst = cont["name_lst"]
    idx_lst = cont["idx_lst"]
    front_sim_param_lst = cont["front_sim_param_lst"]
    back_sim_param_lst = cont["back_sim_param_lst"]
    return name_lst, idx_lst, front_sim_param_lst, back_sim_param_lst


def get_front_data():
    name_lst, idx_lst, front_sim_param_lst, back_sim_param_lst = get_data()
    warp_lst = []
    weft_lst = []
    bias_lst = []
    for i in front_sim_param_lst:
        warp_lst.append(i['warp'])
        weft_lst.append(i['weft'])
        bias_lst.append(i['bias'])

    return name_lst, idx_lst, warp_lst, weft_lst, bias_lst


if __name__ == "__main__":
    name_lst, idx_lst, warp_lst, weft_lst, bias_lst = get_front_data()

    output_dir = "output"
    if osp.exists(output_dir) == False:
        os.makedirs(output_dir)
    max_axis = max(max(warp_lst), max(weft_lst), max(bias_lst))
    min_axis = min(min(warp_lst), min(weft_lst), min(bias_lst))

    for ii in tqdm(range(0, 360, 1)):
        fig = plt.figure()
        ax = Axes3D(fig)
        ax.set_xlabel("warp")
        ax.set_ylabel("weft")
        ax.set_zlabel("bias")
        ax.set_xbound(min_axis, max_axis)
        ax.set_ybound(min_axis, max_axis)
        ax.set_zbound(min_axis, max_axis)
        ax.view_init(elev=10., azim=ii)
        ax.scatter3D(warp_lst, weft_lst, bias_lst)
        save_name = f"{output_dir}/{ii}.png"
        plt.savefig(save_name)
        plt.cla()
        plt.clf()
        plt.close('all')