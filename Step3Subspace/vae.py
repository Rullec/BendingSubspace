import torch
import numpy as np
import torch.optim as optim
import matplotlib.pyplot as plt
import torch.nn as nn
import torchvision.datasets as dset
from torch.utils.data import DataLoader
from torchvision import transforms
from torch.functional import F
from torch.autograd import Variable


def get_dataloader():
    data_root = "./data"

    train_set = dset.MNIST(
        root=data_root,
        train=True,
        transform=transforms.Compose([
            transforms.ToTensor(),
            #    transforms.Normalize(mean=128.0, std=255.0)
        ]),
        download=True)
    test_set = dset.MNIST(
        root=data_root,
        train=False,
        transform=transforms.Compose([
            transforms.ToTensor(),
            #   transforms.Normalize(mean=128.0, std=255.0)
        ]),
        download=True)
    # indices = torch.randperm(len(train_set))[:500]
    # train_set = torch.utils.data.Subset(train_set, indices)

    # RandomSu
    # 1. dataloader
    batch_size = 32
    train_loader = DataLoader(train_set,
                              batch_size,
                              shuffle=True,
                              num_workers=2)
    test_loader = DataLoader(test_set, batch_size, shuffle=True)
    return train_loader, test_loader
    # 1. construct the net

    # 2. prepare the dataloader (and MNIST dataset)
    # 3. fit and check the error


# reparametrization trick
def compute_latent(mu, sigma):
    return torch.distributions.normal(mu, sigma)


def kl_divergence(logvar, mu):
    '''
    mu in [-inf, inf]
    logvar = log (sigma^2) in [-inf, inf]
    '''
    # kl_div = torch.log(sigma) + 1 - mu**2 - sigma**2
    kl_div = logvar + 1 - mu**2 - torch.exp(logvar)
    kl_div *= -0.5
    kl_div = torch.sum(kl_div)
    return kl_div


def reconstruction_loss(pred, true):
    # return nn.MSELoss(reduce='sum')(pred, true)
    return F.binary_cross_entropy(pred, true, reduction='sum')


class Encoder(nn.Module):
    def __init__(self, latent_dim):
        super(Encoder, self).__init__()


class Decoder(nn.Module):
    def __init__(self):
        super(Encoder, self).__init__()


class Net(nn.Module):
    def __init__(self, latent_dim=2):
        super(Net, self).__init__()
        self.latent_dim = latent_dim
        feature_dim = 576
        self.encConv1 = nn.Conv2d(1, 8, 3, 2)
        self.encConv2 = nn.Conv2d(8, 16, 3, 2)
        self.encFC1 = nn.Linear(feature_dim, self.latent_dim)
        self.encFC2 = nn.Linear(feature_dim, self.latent_dim)

        self.decFC1 = nn.Linear(self.latent_dim, feature_dim)
        # self.convT1 = nn.ConvTranspose2d(16, 8, 3, 2, output_padding=1)
        # self.convT2 = nn.ConvTranspose2d(8, 1, 3, 2, output_padding=1)
        self.convT1 = nn.ConvTranspose2d(16, 8, 3, 2)
        self.convT2 = nn.ConvTranspose2d(8, 1, 3, 2, output_padding=1)

    def encoder(self, x):
        # print(f"input img {x.shape}")
        x = F.relu(self.encConv1(x))
        # print(f"after conv1 {x.shape}")
        x = F.relu(self.encConv2(x))
        # print(f"after conv2 {x.shape}")
        # x = F.relu(self.encConv3(x))
        # print(f"after conv3 {x.shape}")
        # exit()
        x = x.view(x.shape[0], -1)

        mu = self.encFC1(x)
        logvar = self.encFC2(x)
        return mu, logvar

    def reparameterize(self, mu, logvar):
        '''
        Given the \mu and \log \sigma^2, sampling from this normal distribution
        '''
        std = torch.exp(logvar / 2)
        eps = torch.randn_like(std)
        return mu + eps * std

    def decoder(self, x):
        x = F.relu(self.decFC1(x))
        x = x.reshape(-1, 16, 6, 6)
        x = F.relu(self.convT1(x))
        # print(f"after convT1 {x.shape}")
        '''
            For the image, the output is all possible
        '''
        x = F.sigmoid(self.convT2(x))
        # print(f"after convT2 {x.shape}")
        return x

    def forward(self, x):
        mu, logvar = self.encoder(x)
        z = self.reparameterize(mu, logvar)
        out = self.decoder(z)
        return out, mu, logvar


def train():
    train_loader, test_loader = get_dataloader()
    # net = Net()
    net = torch.load("epoch-550")
    num_epoch = 1000
    import time
    optimizer = optim.Adam(net.parameters(), lr=1e-4)
    save_epoch = 50
    for epoch in range(num_epoch):
        total_kl_loss = 0
        total_reconstr_loss = 0
        total_loss = 0
        num_items = 0
        st = time.time()
        net.train()
        for img, label in train_loader:
            optimizer.zero_grad()
            out, mu, logvar = net(img)
            # print(f"raw img shape {img.shape}, out shape {out.shape}")
            # print(torch.max(img), torch.min(img))
            # exit()
            # exit()
            num_items += out.shape[0]
            kl_loss = kl_divergence(logvar, mu)
            reconstr_loss = reconstruction_loss(out, img)
            loss = 0.5 * kl_loss + 0.5 * reconstr_loss
            loss.backward()
            optimizer.step()
            total_kl_loss += kl_loss.item()
            total_reconstr_loss += reconstr_loss.item()
            total_loss += loss.item()

        total_kl_loss /= num_items
        total_reconstr_loss /= num_items
        total_loss /= num_items
        ed = time.time()
        print(
            f"epoch {epoch} kl {total_kl_loss} recons {total_reconstr_loss} loss {total_loss} cost {ed - st}s"
        )

        if epoch % 5 == 0:
            out = out.detach().cpu().numpy()
            img = img.detach().cpu().numpy()
            # out = out * 255 + 128
            # img = img * 255 + 128
            import matplotlib.pyplot as plt
            for i in range(6):
                plt.subplot(2, 6, 2 * i + 1)
                plt.imshow(img[i][0])
                plt.title(f"raw img {i}")
                plt.subplot(2, 6, 2 * i + 2)
                plt.imshow(out[i][0])
                plt.title(f"pred img {i}")
            # plt.show()
            plt.savefig(f"epoch-{epoch}.png")
            plt.cla()
            plt.clf()
            plt.close('all')
            # exit()
        if epoch % save_epoch == 0:
            torch.save(net, f"epoch-{epoch}")


def generate(decoder, z_min, z_max, samples=5):
    assert len(z_min) == 2
    assert len(z_max) == 2
    print(f"z min {z_min}")
    print(f"z max {z_max}")
    # begin to do cartesian product and get the reparmertized points
    z0_lst = list(np.linspace(z_min[0], z_max[0], samples))
    z1_lst = list(np.linspace(z_min[1], z_max[1], samples))

    id = 1
    for z0 in z0_lst:
        for z1 in z1_lst:
            plt.subplot(len(z0_lst), len(z1_lst), id)
            id += 1
            img = decoder(torch.Tensor([z0, z1]))
            img = img.detach().cpu().numpy()[0]
            img = np.moveaxis(img, 0, 2)
            plt.xticks([])
            plt.yticks([])
            plt.imshow(img)
            # plt.title(f"({z0:.1f}, {z1:.1f})")

    plt.show()


def uniform_sampling_from_the_dataset_and_check_the_inference(
        net, img_tensor, label_tensor):
    label_tensor = list(label_tensor)
    img_tensor = list(img_tensor)
    print(len(label_tensor), label_tensor[0])
    print(len(img_tensor), img_tensor[0].shape)
    img_tensor = [
        x
        for _, x, in sorted(zip(label_tensor, img_tensor), key=lambda x: x[0])
    ]
    # for a, b in sorted(zip(label_tensor, img_tensor)):
    #     print(b)
    label_tensor = sorted(label_tensor)

    selected_idx_lst = [[] for i in range(10)]
    for idx in range(len(label_tensor)):
        if len(selected_idx_lst[label_tensor[idx]]) <= 3:
            selected_idx_lst[label_tensor[idx]].append(idx)

    img_id = 1
    for cur_lst in selected_idx_lst:
        for id in cur_lst:
            label = label_tensor[id]
            img = img_tensor[id]
            # plt.subplot(10, 10, img_id)
            # plt.imshow(np.moveaxis(img, 0, 2))
            # plt.title(f"{label} raw")
            # img_id += 1
            mu, logvar = net.encoder(torch.Tensor([img]))
            # sample 3 times
            for i in range(1):
                z = net.reparameterize(mu, logvar)
                new_img = net.decoder(z).detach().numpy()

                plt.subplot(7, 7, img_id)
                plt.imshow(np.moveaxis(new_img[0], 0, 2))
                plt.title(f"{label} gen")
                img_id += 1
    plt.show()


def test():
    train_loader, test_loader = get_dataloader()
    net = torch.load("epoch-550")
    # net = Net()
    img_tensor = []
    label_tensor = []
    mu_tensor = []
    logvar_tensor = []
    z_tensor = []
    iter = 0
    for img, label in train_loader:
        mu, logvar = net.encoder(img)
        z = net.reparameterize(mu, logvar)
        z_tensor.append(z)
        img_tensor.append(img)
        label_tensor.append(label)
        mu_tensor.append(mu)
        logvar_tensor.append(logvar)
        iter += 1
        if iter >= 10:
            break

    img_tensor = torch.cat(img_tensor).detach().numpy()
    label_tensor = torch.cat(label_tensor).detach().numpy()
    mu_tensor = torch.cat(mu_tensor).detach().numpy()
    logvar_tensor = torch.cat(logvar_tensor).detach().numpy()
    z_tensor = torch.cat(z_tensor).detach().numpy()
    z_min = np.min(z_tensor, axis=0)
    z_max = np.max(z_tensor, axis=0)

    uniform_sampling_from_the_dataset_and_check_the_inference(
        net, img_tensor, label_tensor)
    exit()
    # generate(net.decoder, z_min, z_max, samples=20)
    # exit()
    unique_elements, counts = np.unique(label_tensor, return_counts=True)
    print(f"frequency of unique values of the said array:")
    print(unique_elements, counts)
    print(img_tensor.shape)
    print(label_tensor.shape)
    print(mu_tensor.shape)
    print(logvar_tensor.shape)
    plt.subplot(2, 2, 1)
    plt.scatter(z_tensor[:, 0],
                z_tensor[:, 1],
                s=2,
                c=label_tensor,
                cmap='hsv')
    plt.colorbar()
    plt.grid()
    plt.title("z")

    plt.subplot(2, 2, 2)
    plt.scatter(mu_tensor[:, 0],
                mu_tensor[:, 1],
                s=2,
                c=label_tensor,
                cmap='hsv')
    plt.colorbar()
    plt.grid()
    plt.title("mu")

    plt.subplot(2, 2, 3)
    plt.scatter(logvar_tensor[:, 0],
                logvar_tensor[:, 1],
                s=2,
                c=label_tensor,
                cmap='hsv')
    plt.colorbar()
    plt.grid()
    plt.title("logvar")
    plt.show()


if __name__ == "__main__":
    test()