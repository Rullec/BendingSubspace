import pickle as pkl
import numpy as np
import os.path as osp
import sys

sys.path.append(osp.join(osp.dirname(__file__), '..'))
from draw_3d import Draw3D


def remove_symmetric_prop(raw_props):
    # for i in list(raw_props):
    #     print(i)
    # warp, weft, bias
    for i in range(raw_props.shape[0]):
        warp = raw_props[i][0]
        weft = raw_props[i][1]
        if warp < weft:
            raw_props[i][0] = weft
            raw_props[i][1] = warp
    # print(raw_props.shape)
    new_props = np.unique(raw_props, axis=0)
    # print(new_props.shape)
    return new_props


if __name__ == "__main__":
    pkl_path = "20210910_sampling.pkl"
    with open(pkl_path, 'rb') as f:
        raw_props = pkl.load(f)
    int_props = np.rint(raw_props)

    remove_symmetric_prop(int_props)
    drawer = Draw3D()
    drawer.add_points(raw_props[:, 0],
                      raw_props[:, 1],
                      raw_props[:, 2],
                      color='blue',
                      alpha=0.1,
                      label="float")
    drawer.add_points(int_props[:, 0],
                      int_props[:, 1],
                      int_props[:, 2],
                      color='red',
                      alpha=0.1,
                      label="int")
    drawer.draw()

    output_pkl_path = "20210910_sampling_new.pkl"
    with open(output_pkl_path, 'wb') as f:
        pkl.dump(int_props, f)
        print(f"output new props to {output_pkl_path}")