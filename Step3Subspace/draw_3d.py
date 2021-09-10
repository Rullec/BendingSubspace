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


class Draw3D:
    def __init__(self):
        self.x_lst_group = []
        self.y_lst_group = []
        self.z_lst_group = []
        self.color_group = []
        self.alpha_group = []
        self.label_group = []
        self.func = None

    def add_points(self, x_lst, y_lst, z_lst, color='blue', alpha=1, label=""):
        self.x_lst_group.append(x_lst)
        self.y_lst_group.append(y_lst)
        self.z_lst_group.append(z_lst)
        self.color_group.append(color)
        self.alpha_group.append(alpha)
        self.label_group.append(label)

    def get_max_range(self):
        max_value = np.max([
            np.max([np.max(i) for i in self.x_lst_group]),
            np.max([np.max(i) for i in self.y_lst_group]),
            np.max([np.max(i) for i in self.z_lst_group]),
        ])
        min_value = np.min([
            np.min([np.min(i) for i in self.x_lst_group]),
            np.min([np.min(i) for i in self.y_lst_group]),
            np.min([np.min(i) for i in self.z_lst_group]),
        ])

        return min_value, max_value

    def draw(self):
        fig = plt.figure()
        ax = Axes3D(fig)
        ax.set_xlabel("warp")
        ax.set_ylabel("weft")
        ax.set_zlabel("bias")
        low_lim, high_lim = self.get_max_range()
        ax.set_xbound(low_lim, high_lim)
        ax.set_ybound(low_lim, high_lim)
        ax.set_zbound(low_lim, high_lim)
        if self.func is not None:
            print("set pick event")
            fig.canvas.mpl_connect('pick_event', self.func)
        # ax.view_init(elev=10., azim=ii)
        for i in range(len(self.x_lst_group)):
            ax.scatter3D(self.x_lst_group[i],
                         self.y_lst_group[i],
                         self.z_lst_group[i],
                         color=self.color_group[i],
                         alpha=self.alpha_group[i],
                         label=self.label_group[i])
            # for j in range(len(self.x_lst_group[i])):
            #     ax.text3D(self.x_lst_group[i][j],
            #                 self.y_lst_group[i][j],
            #                 self.z_lst_group[i][j], str(j))
        plt.legend()
        plt.show()

    def draw_gif(self):
        fig = plt.figure()
        for ii in tqdm(range(0, 360, 2)):
            ax = Axes3D(fig)
            ax.set_xlabel("warp")
            ax.set_ylabel("weft")
            ax.set_zlabel("bias")
            low_lim, high_lim = self.get_max_range()
            ax.set_xbound(low_lim, high_lim)
            ax.set_ybound(low_lim, high_lim)
            ax.set_zbound(low_lim, high_lim)
            ax.view_init(elev=10., azim=ii)
            for i in range(len(self.x_lst_group)):
                ax.scatter3D(self.x_lst_group[i],
                             self.y_lst_group[i],
                             self.z_lst_group[i],
                             color=self.color_group[i],
                             alpha=self.alpha_group[i],
                             label=self.label_group[i])
            plt.legend()
            plt.savefig(f"output/{ii}.png")
            plt.cla()
            plt.clf()

    def set_pick_callback(self, func):
        self.func = func


if __name__ == "__main__":
    name_lst, idx_lst, warp_lst, weft_lst, bias_lst = get_front_data()

    output_dir = "output"
    if osp.exists(output_dir) == False:
        os.makedirs(output_dir)
    max_axis = max(max(warp_lst), max(weft_lst), max(bias_lst))
    min_axis = min(min(warp_lst), min(weft_lst), min(bias_lst))

    drawer = Draw3D()
    drawer.add_points(warp_lst, weft_lst, bias_lst)
    drawer.draw()
    # for ii in tqdm(range(0, 360, 1)):
    #     fig = plt.figure()
    #     ax = Axes3D(fig)
    #     ax.set_xlabel("warp")
    #     ax.set_ylabel("weft")
    #     ax.set_zlabel("bias")
    #     ax.set_xbound(min_axis, max_axis)
    #     ax.set_ybound(min_axis, max_axis)
    #     ax.set_zbound(min_axis, max_axis)
    #     ax.view_init(elev=10., azim=ii)
    #     ax.scatter3D(warp_lst, weft_lst, bias_lst)
    #     save_name = f"{output_dir}/{ii}.png"
    #     plt.savefig(save_name)
    #     plt.cla()
    #     plt.clf()
    #     plt.close('all')