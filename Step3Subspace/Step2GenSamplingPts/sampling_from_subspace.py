import warnings

warnings.simplefilter(action='ignore', category=FutureWarning)

import os.path as osp
import sys
import torch

sys.path.append(osp.join(osp.dirname(__file__), '..'))
from Step1VAEtraining.dataloader import get_dataloader
from Step1VAEtraining.vae_subspace import get_all_data, sample_new_data, Net
from draw_3d import Draw3D
from Step1VAEtraining.dataloader import ToyDataset
import numpy as np
from tqdm import tqdm


def calculate_sample_point_from_point_cloud(raw_point_cloud: np.ndarray,
                                            min_dist_thre=2):
    assert type(raw_point_cloud) == np.ndarray, type(raw_point_cloud)
    raw_point_cloud = list(raw_point_cloud)
    sampled_pts = []
    for _idx in tqdm(range(len(raw_point_cloud))):
        # print(f"----------")
        raw_pt = raw_point_cloud[_idx]
        if len(sampled_pts) == 0:
            sampled_pts.append(raw_pt)
        else:
            diff = raw_pt - np.array(sampled_pts)
            # print(f"raw pt {raw_pt}")
            # print(f"sampled {sampled_pts}")
            # print(f"diff {diff} {diff.shape}")
            diff_norm = np.linalg.norm(diff, axis=1)
            # print(f"diff norm {diff_norm} {diff_norm.shape}")
            min_dist = np.min(diff_norm)
            if min_dist > min_dist_thre:
                sampled_pts.append(raw_pt)
            # else:
            #     print(
            #         f"dist {min_dist} < min_dist_thre {min_dist_thre}, ignore {len(sampled_pts)}"
            #     )
            # if len(sampled_pts) > 2:
            #     break
    sampled_pts = np.array(sampled_pts)
    return sampled_pts


def clip_the_sampled_props(value_lst: np.ndarray):
    valid_idx = []
    min_value = 1
    max_value = 100
    for i in range(value_lst.shape[0]):
        if np.max(value_lst[i]) <= max_value and np.min(
                value_lst[i]) >= min_value:
            valid_idx.append(i)
    from operator import itemgetter
    return np.array(list(itemgetter(*valid_idx)(list(value_lst))))


def sampling_from_uniform_latence(model: Net,
                                  dataset_input: torch.Tensor,
                                  scale=0):
    mu, logvar = model.encoder(dataset_input)
    z = torch.cat([model.reparameterize(mu, logvar)
                   for _ in range(10)]).detach().numpy()
    max_z = np.max(z, axis=0)
    min_z = np.min(z, axis=0)
    for i in range(len(max_z)):
        if max_z[i] > 0:
            max_z[i] *= (1 + scale)
        else:
            max_z[i] *= (1 - scale)
        if min_z[i] > 0:
            min_z[i] *= (1 + scale)
        else:
            min_z[i] *= (1 - scale)

    num_of_samples = 50
    from itertools import product

    sampled_z = list(
        product(*[
            list(np.linspace(min_z[i], max_z[i], num_of_samples))
            for i in range(3)
        ]))
    # product(data)

    value_lst = model.decoder(
        torch.Tensor(sampled_z)).detach().numpy() * ToyDataset.normalize_amp
    value_lst = calculate_sample_point_from_point_cloud(value_lst)

    print(f"get {value_lst.shape[0]} props from sampling")
    value_lst = clip_the_sampled_props(value_lst)
    print(f"get {value_lst.shape[0]} props after clip")
    # drawer = Draw3D()
    # drawer.add_points(value_lst[:, 0], value_lst[:, 1], value_lst[:, 2])
    # drawer.draw()
    # exit()
    return value_lst


def sampling_from_dataset_density(net, feature_tensor):
    new_feature_tensor = ToyDataset.unnormalize(
        sample_new_data(net,
                        ToyDataset.normalize(feature_tensor),
                        sample_density=2000).detach().numpy())

    new_feature_tensor = calculate_sample_point_from_point_cloud(
        new_feature_tensor, min_dist_thre=2)
    new_feature_tensor = clip_the_sampled_props(new_feature_tensor)
    return new_feature_tensor


if __name__ == "__main__":
    # 1. get the model path
    # vae_model_path = "../output/20210909_version1.pkl"
    vae_model_path = "../output/20210910_version2.pkl"
    net = torch.load(vae_model_path)
    assert osp.exists(vae_model_path)
    dataset_root_dir = "../Step1VAEtraining/dataset.log"
    assert osp.exists(dataset_root_dir)

    # 2. get the data
    train_loader, _ = get_dataloader(dataset_root_dir)
    feature_tensor, name_tensor = get_all_data(train_loader, None)
    feature_tensor *= ToyDataset.normalize_amp

    # new_feature_tensor = sampling_from_uniform_latence(
    #     net, ToyDataset.normalize(torch.Tensor(feature_tensor)))

    new_feature_tensor = sampling_from_dataset_density(net, feature_tensor)

    drawer = Draw3D()
    print(feature_tensor.shape)
    print(new_feature_tensor.shape)
    drawer.add_points(feature_tensor[:, 0],
                      feature_tensor[:, 1],
                      feature_tensor[:, 2],
                      color='blue',
                      alpha=1,
                      label="dataset")
    drawer.add_points(new_feature_tensor[:, 0],
                      new_feature_tensor[:, 1],
                      new_feature_tensor[:, 2],
                      color='red',
                      alpha=0.2,
                      label="generated")
    # drawer.draw()
    drawer.draw_gif()
    # 3. uniform sampling & sampling from the data point, check the result
    # make it bigger

    # 4. select the best sampling method, do sampling
    # 5. filtering, output the sampling property file