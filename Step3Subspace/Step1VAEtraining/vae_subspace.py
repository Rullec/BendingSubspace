import warnings

warnings.simplefilter(action='ignore', category=FutureWarning)
import os
import torch
import numpy as np
import torch.optim as optim
from torch.utils.data import Dataset, DataLoader
from .dataloader import *
import torch.nn as nn
from torch.functional import F
import os.path as osp
import datetime
import sys

sys.path.append(osp.join(osp.dirname(__file__), ".."))
from draw_3d import Draw3D
from torch.utils.tensorboard import SummaryWriter

tfb_writer = SummaryWriter("./log/tensorboard_log/" +
                           datetime.datetime.now().strftime("%Y%m%d-%H%M%S") +
                           "/")

np.random.seed(0)
torch.manual_seed(0)


class Net(nn.Module):
    def __init__(self, input_dim=3, latent_dim=3):
        super(Net, self).__init__()
        self.latent_dim = latent_dim
        self.encFc1 = nn.Linear(input_dim, 64)
        self.encFc2 = nn.Linear(64, 32)

        self.encFC_mu = nn.Linear(32, self.latent_dim)
        self.encFC_logvar = nn.Linear(32, self.latent_dim)

        self.decFC1 = nn.Linear(self.latent_dim, 32)
        self.decFC2 = nn.Linear(32, 64)
        self.decFC3 = nn.Linear(64, input_dim)

    def encoder(self, x):
        x = F.relu(self.encFc1(x))
        x = F.relu(self.encFc2(x))
        mu = self.encFC_mu(x)
        logvar = self.encFC_logvar(x)
        return mu, logvar

    def reparameterize(self, mu, logvar):
        '''
        Given the \mu and \log \sigma^2, sampling from this normal distribution
        '''
        std = torch.exp(logvar / 2)
        # print(f"std {std}")
        # print(f"logvar {logvar}")
        eps = torch.randn_like(std)
        # print(f"eps {eps}")
        return mu + eps * std

    def decoder(self, x):
        x = F.relu(self.decFC1(x))
        x = F.relu(self.decFC2(x))
        x = self.decFC3(x)
        return x

    def forward(self, x):
        mu, logvar = self.encoder(x)
        z = self.reparameterize(mu, logvar)
        out = self.decoder(z)
        return out, mu, logvar


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
    return F.mse_loss(pred, true, reduce='sum')
    # return F.binary_cross_entropy(pred, true, reduce='sum')


    # return nn.MSELoss(reduce='sum')(pred, true)
    # return F.binary_cross_entropy(pred, true, reduction='sum')
def train(kl_weight_begin_epoch, kl_weight_max, weight_path=None):
    output_dir = "output"
    if osp.exists(output_dir) == False:
        os.makedirs(output_dir)
    train_loader, test_loader = get_dataloader()
    if weight_path is not None and osp.exists(weight_path):
        net = torch.load(weight_path)
        # net = torch.load("output/epoch-5000.pkl")
    else:
        net = Net()
    num_epoch = 15001
    import time
    optimizer = optim.Adam(net.parameters(), lr=1e-4)
    save_epoch = 100
    output_epoch = 100

    def calc_kl_weight(epoch):
        if epoch > kl_weight_begin_epoch:
            prop = (epoch - kl_weight_begin_epoch) / 3000
            prop = min(prop, 1)
            return prop * kl_weight_max
        else:
            return 0
        # else:
        #     return 0

    for epoch in range(num_epoch):
        total_kl_loss = 0
        total_reconstr_loss = 0
        total_loss = 0
        num_items = 0
        st = time.time()
        net.train()
        max_diff = -1
        kl_weight = calc_kl_weight(epoch)
        for img, label in train_loader:
            optimizer.zero_grad()
            out, mu, logvar = net(img)
            num_items += out.shape[0]
            max_diff = max(torch.max(torch.abs(out - img)), max_diff)
            # kl_loss = kl_divergence(logvar, mu)
            kl_loss = kl_divergence(logvar, mu)
            reconstr_loss = reconstruction_loss(out, img)
            loss = kl_weight * kl_loss + 0.5 * reconstr_loss
            loss.backward()
            optimizer.step()
            total_kl_loss += kl_loss.item()
            total_reconstr_loss += reconstr_loss.item()
            total_loss += loss.item()

        total_kl_loss /= num_items
        total_reconstr_loss /= num_items
        total_loss /= num_items
        max_diff *= train_loader.dataset.normalize_amp
        ed = time.time()
        tfb_writer.add_scalar("kl_loss", total_kl_loss, epoch)
        tfb_writer.add_scalar("recons_loss", total_reconstr_loss, epoch)
        tfb_writer.add_scalar("kl_weight", kl_weight, epoch)
        tfb_writer.add_scalar("max_diff", max_diff, epoch)
        # -------- begin to do evaluation -------
        # net.eval()
        # num_items_eval = 0
        # total_kl_loss_eval = 0
        # total_reconstr_loss_eval = 0
        # total_loss_eval = 0

        # for _idx, val in enumerate(test_loader):
        #     img, label = val
        #     out, mu, logvar = net(img)
        #     num_items_eval += out.shape[0]
        #     kl_loss = kl_divergence(logvar, mu)
        #     reconstr_loss = reconstruction_loss(out, img)
        #     loss = kl_weight(epoch, num_epoch) * kl_loss + 0.5 * reconstr_loss
        #     total_kl_loss_eval += kl_loss.item()
        #     total_reconstr_loss_eval += reconstr_loss.item()
        #     total_loss_eval += loss.item()
        # total_kl_loss_eval /= num_items
        # total_reconstr_loss_eval /= num_items
        # total_loss_eval /= num_items

        if epoch % output_epoch == 0:
            # print(
            #     f"epoch {epoch} kl {total_kl_loss:.3e} recons {total_reconstr_loss:.3e} val loss {total_loss_eval:.3f} cost {ed - st:.3f}s"
            # )
            print(
                f"epoch {epoch} kl {total_kl_loss:.3f} recons {total_reconstr_loss:.5f} max_diff {max_diff:.3f} cost {ed - st:.4f}s"
            )
        if epoch % save_epoch == 0:
            torch.save(net, osp.join(output_dir, f"epoch-{epoch}.pkl"))


def get_all_data(train_dataloader, test_dataloader):
    img_tensor = []
    label_tensor = []

    for img, label in train_dataloader:
        img_tensor.append(img)
        label_tensor += label
    if test_dataloader is not None:
        for img, label in test_dataloader:
            img_tensor.append(img)
            label_tensor += label

    img_tensor = torch.cat(img_tensor).detach().numpy()
    # label_tensor = torch.cat(label_tensor).detach().numpy()
    return img_tensor, label_tensor


def sample_new_data(model, old_feature, sample_density=20):
    old_feature = torch.Tensor(old_feature)
    mu, logvar = model.encoder(old_feature)
    # print(f"log var {logvar}")
    # new_data_lst = [
    #     model.decoder(model.reparameterize(mu, logvar)) for _ in range(10)
    # ]
    new_data_lst = []
    for _ in range(sample_density):
        # print(f"mu {mu[0]} logvar {logvar[0]}")
        z = model.reparameterize(mu, logvar)
        # print(f"z {z[0,:]}")
        feature = model.decoder(z)
        # print(f"feature {feature[0, :]}")
        new_data_lst.append(feature)
    new_data = torch.cat(new_data_lst)
    return new_data


def test(pkl):
    model = torch.load(pkl)
    feature_lst, label_lst = get_all_data(*(get_dataloader()))
    drawer = Draw3D()
    drawer.add_points(feature_lst[:, 0],
                      feature_lst[:, 1],
                      feature_lst[:, 2],
                      color='blue',
                      alpha=1,
                      label="raw")
    new_data = sample_new_data(model, feature_lst)
    new_data = new_data.detach().numpy()
    drawer.add_points(new_data[:, 0],
                      new_data[:, 1],
                      new_data[:, 2],
                      color='red',
                      alpha=0.1,
                      label="gen")
    drawer.draw_gif()
    # drawer.draw()
    # from tqdm import tqdm
    # import os
    # output_dir = "output"
    # if os.path.exists(output_dir) == False:
    #     os.makedirs(output_dir)
    # for ii in tqdm(range(0, 360, 1)):
    #     fig = plt.figure()
    #     ax = Axes3D(fig)
    #     ax.set_xlabel("warp")
    #     ax.set_ylabel("weft")
    #     ax.set_zlabel("bias")
    #     ax.view_init(elev=10., azim=ii)
    #     ax.scatter3D(new_data[:, 0],
    #                  new_data[:, 1],
    #                  new_data[:, 2],
    #                  color='red',
    #                  label='gen data',
    #                  alpha=0.1)
    #     ax.scatter3D(feature_lst[:, 0],
    #                  feature_lst[:, 1],
    #                  feature_lst[:, 2],
    #                  color='blue',
    #                  label='raw data',
    #                  alpha=1)
    #     save_name = f"{output_dir}/{ii}.png"
    #     plt.legend()
    #     plt.savefig(save_name)
    #     plt.cla()
    #     plt.clf()
    #     plt.close('all')


import argparse
if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument("--test",
                        type=str,
                        default=None,
                        help="test the model weight")
    parser.add_argument("--train",
                        type=str,
                        default=None,
                        help="the init model weight")
    args = parser.parse_args()

    if args.test is not None:
        test(args.test)
    else:
        # train(kl_weight_max=1e-3)
        # print(args.train)
        # exit()
        train(kl_weight_begin_epoch=4000,
              kl_weight_max=1e-4,
              weight_path=args.train)
