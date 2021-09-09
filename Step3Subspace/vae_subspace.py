import pickle as pkl
import torch
import numpy as np
from torch.utils.data import Dataset, DataLoader


class ToyDataset(Dataset):
    def __init__(self) -> None:
        pkl_path = r"D:\Projects\Subspace\Step2FromGBToSim\output\bending_sim_param.pkl"
        with open(pkl_path, 'rb') as f:
            cont = pkl.load(f)
        # print(cont.keys())

        #  cont["back_sim_param_lst"]
        front_sim = cont["front_sim_param_lst"]
        self.name_lst = cont["name_lst"]
        self.data_lst = np.array([[i['warp'], i['weft'], i['bias']]
                                  for i in front_sim])

    def __len__(self):
        return self.data_lst.shape[0]

    def __getitem__(self, index):
        return self.data_lst[index], self.name_lst[index]


def get_dataloader():
    dst = ToyDataset()
    len_dst = len(dst)
    rand_indices = torch.randperm(len_dst)
    train_indices = rand_indices[:int(0.8 * rand_indices)]
    test_indices = rand_indices[int(0.8 * rand_indices):]
    train_set = torch.utils.data.Subset(dst, train_indices)
    test_set = torch.utils.data.Subset(dst, test_indices)

    batch_size = 4
    train_loader = DataLoader(train_set, batch_size, shuffle=True)
    test_loader = DataLoader(train_set, batch_size, shuffle=True)
    return train_loader, test_loader


if __name__ == "__main__":
    train_loader, test_loader = get_dataloader()