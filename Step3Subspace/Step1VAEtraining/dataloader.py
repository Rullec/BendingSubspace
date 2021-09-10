from torch.utils.data import Dataset, DataLoader
import torch
import numpy as np
import os.path as osp


class ToyDataset(Dataset):
    normalize_amp = 10

    def __init__(self, data_dir, phase_train, full_data=False) -> None:
        train_file = "train.txt"
        test_file = "test.txt"

        if full_data == False:
            rec_file = train_file if phase_train == True else test_file
            rec_file = osp.join(data_dir, rec_file)
            with open(rec_file, 'r') as f:
                cont = f.readlines()
        elif full_data == True:
            print(f"load full dataset")
            cont = []
            with open(osp.join(data_dir, train_file), 'r') as f:
                cont += f.readlines()
            with open(osp.join(data_dir, test_file), 'r') as f:
                cont += f.readlines()
        '''
        the format of the record file
        '''
        self.name_lst = []
        self.data_lst = []
        for line in cont:
            line = line.strip()
            if line[0] == '#':
                continue
            warp, weft, bias, face, _, name = line.split()
            data = np.array(
                [float(warp), float(weft),
                 float(bias)], dtype=np.float32)
            name = f"{name}_{face}"
            if name == "威化26度牛仔_front" or name == "平纹_front" or name == "牛皮_front" or name == "棉天丝牛仔_front":
                print(f"[warn] ignore {name}")
                continue

            self.name_lst.append(name)
            self.data_lst.append(data)

        self.data_lst = np.array(self.data_lst)
        # self.data_lst = self.data_lst[:1000]
        # self.name_lst = self.name_lst[:1000]

    def normalize(x):
        return x / ToyDataset.normalize_amp

    def unnormalize(x):
        return x * ToyDataset.normalize_amp

    # def normalize(x):
    #     return x

    # def unnormalize(x):
    #     return x

    def __len__(self):
        return self.data_lst.shape[0]

    def __getitem__(self, index):
        return ToyDataset.normalize(self.data_lst[index]), self.name_lst[index]


def get_dataloader(data_dir="dataset.log",
                   batch_size=32,
                   load_full_data_in_train_loader=True):
    train_set = ToyDataset(data_dir=data_dir,
                           phase_train=True,
                           full_data=load_full_data_in_train_loader)
    test_set = ToyDataset(data_dir=data_dir, phase_train=False)
    train_loader = DataLoader(train_set, batch_size, shuffle=True)
    test_loader = DataLoader(test_set, batch_size, shuffle=True)
    return train_loader, test_loader


def get_all_data(train_dataloader, test_dataloader=None):
    img_tensor = []
    label_tensor = []

    for img, label in train_dataloader:
        img_tensor.append(img)
        label_tensor.append(label)
    for img, label in test_dataloader:
        img_tensor.append(img)
        label_tensor.append(label)

    img_tensor = torch.cat(img_tensor).detach().numpy()
    # label_tensor = torch.cat(label_tensor).detach().numpy()
    return img_tensor, label_tensor