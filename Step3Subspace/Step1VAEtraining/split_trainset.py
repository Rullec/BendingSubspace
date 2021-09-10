import numpy as np
import shutil
import os
import os.path as osp
import pickle as pkl


def output_dataset(value_lst, name_lst, idx_lst, comment_lst, output_file):

    with open(output_file, 'w') as f:
        # 1. write down the format
        f.write(
            '# warp_value, weft_value, bias_value, comment(face info), idx, name\n'
        )
        # 2. begin to write down the result
        num_of_files = len(name_lst)
        assert len(idx_lst) == num_of_files
        assert len(value_lst) == num_of_files
        for i in range(num_of_files):
            warp, weft, bias = value_lst[i]["warp"], value_lst[i][
                "weft"], value_lst[i]["bias"]
            name = name_lst[i]
            idx = idx_lst[i]
            comment = comment_lst[i]
            cont = "{:.1f} {:.1f} {:.1f} {} {} {}\n".format(
                warp, weft, bias, comment, idx, name)
            f.write(cont)
    print(f"write to {output_file} {num_of_files} items done")


def load_single_pkl(pkl_path):
    # pkl_path = r"D:\Projects\Subspace\Step2FromGBToSim\output\bending_sim_param.pkl"
    assert osp.exists(pkl_path)
    if osp.exists(output_dir):
        shutil.rmtree(output_dir)
    os.makedirs(output_dir)

    # 1. load the value
    with open(pkl_path, 'rb') as f:
        cont = pkl.load(f)
    front_lst = cont["front_sim_param_lst"]
    back_lst = cont["back_sim_param_lst"]
    value_lst = front_lst + back_lst

    comment_lst = [
        'front' if i < len(front_lst) else 'back'
        for i in range(len(value_lst))
    ]
    name_lst = cont["name_lst"]

    name_lst = [
        name_lst[i] if i < len(front_lst) else name_lst[i - len(front_lst)]
        for i in range(len(value_lst))
    ]

    idx_lst = cont["idx_lst"]
    idx_lst = [
        idx_lst[i] if i < len(front_lst) else idx_lst[i - len(front_lst)]
        for i in range(len(value_lst))
    ]

    for idx in range(len(name_lst)):
        name_lst[idx] = name_lst[idx].replace(" ", "_")
    return name_lst, idx_lst, value_lst, comment_lst


import sys

sys.path.append(osp.join(osp.dirname(__file__), ".."))
from draw_3d import Draw3D
if __name__ == "__main__":
    output_dir = "dataset.log"
    pkl_lst = [
        r"D:\Projects\Subspace\Step2FromGBToSim\output\bending_sim_param.pkl",
        r"D:\Projects\Subspace\Step2FromGBToSim\output\bending_sim_param_extended.pkl",
    ]

    value_lst = []
    name_lst = []
    idx_lst = []
    comment_lst = []
    # colors = ["red", "blue"]
    # labels = ["raw", "new"]
    # drawer = Draw3D()
    for _idx, pkl_path in enumerate(pkl_lst):
        name__, idx__, value__, comment__ = load_single_pkl(pkl_path)

        # drawer.add_points([i["warp"] for i in value__],
        #                   [i["weft"] for i in value__],
        #                   [i["bias"] for i in value__],
        #                   label=labels[_idx],
        #                   color=colors[_idx])
        name_lst += name__
        value_lst += value__
        idx_lst += idx__
        comment_lst += comment__
    # drawer.draw()
    # print(len(name_lst))
    # exit()
    # 2. split the train set and the test set
    perc = 0.8
    num_of_train_set = int(len(value_lst) * perc)
    perm = np.random.permutation(len(value_lst))
    train_indices = perm[:num_of_train_set]
    test_indices = perm[num_of_train_set:]

    from operator import itemgetter
    get_item = lambda lst, indices: list(itemgetter(*indices)(lst))

    train_file = osp.join(output_dir, "train.txt")
    test_file = osp.join(output_dir, "test.txt")
    output_dataset(get_item(value_lst, train_indices),
                   get_item(name_lst, train_indices),
                   get_item(idx_lst, train_indices),
                   get_item(comment_lst, train_indices), train_file)
    output_dataset(get_item(value_lst, test_indices),
                   get_item(name_lst, test_indices),
                   get_item(idx_lst, test_indices),
                   get_item(comment_lst, test_indices), test_file)
