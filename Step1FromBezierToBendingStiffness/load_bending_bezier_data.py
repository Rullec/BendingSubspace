import os
import numpy as np
import os.path as osp
from bezier_curve import BezierCurve
from scipy.io import loadmat
from matplotlib import pyplot as plt
from multiprocessing import Pool
from glob import glob
import platform


def get_fabric_subdirs(root_dir):
    subdirs = os.listdir(root_dir)
    subdirs_id_lst = []
    for subdir in subdirs:
        if osp.isdir(osp.join(root_dir, subdir)) == True:
            try:
                # try to convert the dir name to the int
                subdirs_id_lst.append(int(subdir))
            except Exception as e:
                print(f"[warn] ignore \"{subdir}\" fabric subdir")

    subdirs_id_lst = sorted(subdirs_id_lst)
    normal_dirs = [os.path.join(root_dir, str(i)) for i in subdirs_id_lst]
    return normal_dirs


def get_delimiter():
    if platform.system() == "Windows":
        delimiter = "\\"
    elif platform.system() == "Linux":
        delimiter = "/"
    else:
        raise ValueError(platform.system())
    return delimiter


def fetch_fabric_data(dirname):
    delimiter = get_delimiter()
    # 1. validate the data
    assert osp.exists(dirname)
    back_bezier_data = [
        i.split(delimiter)[-1] for i in glob(osp.join(dirname, "Back*.mat"))
    ]
    front_bezier_data = [
        i.split(delimiter)[-1] for i in glob(osp.join(dirname, "Front*.mat"))
    ]
    images = [i.split(delimiter)[-1] for i in glob(osp.join(dirname, "*.JPG"))]

    back_bezier_data.sort(key=lambda x: float(x[5:-4]))
    front_bezier_data.sort(key=lambda x: float(x[6:-4]))
    try:
        images.sort(key=lambda x: float(x[4:-4]))
    except:
        images.sort(key=lambda x: float(x[0:-4]))

    num_back = len(back_bezier_data)
    num_front = len(front_bezier_data)
    num_imgs = len(images)
    assert num_back == num_front

    # the fix method
    # if num_imgs == (num_front + num_back):
    #     import shutil
    #     first_image = images[0]
    #     new_name = first_image
    #     if len(new_name) > 5:
    #         new_name = new_name.replace("DSC", "BEG")
    #     else:
    #         new_name = new_name.replace("0", "-1")
    #     print(f"copy {first_image} to {new_name} as begining")
    #     shutil.copyfile(
    #         osp.join(dirname, first_image),
    #         osp.join(dirname, new_name)
    #     )
    #     num_imgs += 1

    assert num_imgs == (
        num_front + num_back + 1
    ), f"check img number {dirname} failed, num back {num_back} num front {num_front} img {num_imgs}"

    # print(f"validate {dirname} done")

    # 2. build the correspondences between images and bezier curve
    front_image_data = [images[2 * i + 0 + 1] for i in range(num_front)]
    back_image_data = [images[2 * i + 1 + 1] for i in range(num_front)]

    # save_filename = osp.basename(dirname) + ".png"
    # if osp.exists(save_filename) is False:

    #     import matplotlib.pyplot as plt
    #     fig = plt.figure(figsize=(16.0, 10.0))
    #     for i in range(2 * num_front):
    #         plt.subplot(4, 4, i + 1)
    #         plt.imshow(plt.imread(osp.join(dirname, images[i + 1])))

    #     fig.savefig(save_filename)

    return front_image_data, front_bezier_data, back_image_data, back_bezier_data


from skimage import transform as tf
from PIL import Image


def img_warp(raw_img, proj2d_mat):
    '''
        matlab image projective transform matrix
        (dont know the certain direction, need to try)

    '''
    raw_img = np.array(raw_img)

    raw_img_bigger = np.zeros([
        raw_img.shape[0] * int(proj2d_mat[0, 0]),
        raw_img.shape[1] * int(proj2d_mat[1, 1]), raw_img.shape[2]
    ])
    raw_img_bigger[:raw_img.shape[0], :raw_img.shape[1], :] = raw_img
    trans = tf.ProjectiveTransform(np.linalg.inv(proj2d_mat))
    new_img = tf.warp(raw_img_bigger, trans).astype(np.uint8)
    return new_img


def load_bezier_datamat(datamat):
    '''
    return:
        A, B, C, D: the beizer point [in pixel coords]
        img_filename: corresponding image path
        unit_cm: the number of pixels per cm
        proj2d: the proj2d warp mat for this image
    '''
    mat = loadmat(datamat)
    A, B, C, D = np.array(mat['A']), np.array(mat['B']), np.array(
        mat['C']), np.array(mat['D'])
    img_filename = mat['fileName']
    unit_cm = float(np.squeeze(np.array(mat['unitCM'])))
    projective2d = np.array([
        [2.005, 0, 0],
        [0, 2.0, 0],
        [0, 0, 1],
    ])

    return A, B, C, D, unit_cm, img_filename, projective2d


def load_captured_image(path, proj2d):
    assert osp.exists(path) is True
    data = img_warp(Image.open(path), proj2d)
    data = data[::-1, :, :]
    imshow_origin = 'lower'
    return data, imshow_origin


def draw_bezier(root_dir, front_image_data, front_bezier_data, back_image_data,
                back_bezier_data):
    # 1. input: bezier curve and image dir

    output = osp.basename(root_dir) + ".png"
    if osp.exists(output) is True:
        print(f"{output} exists, ignore")
        return

    image_data = front_image_data + back_image_data
    bezier_data = front_bezier_data + back_bezier_data
    fig = plt.figure(figsize=(16.0, 10.0))
    for i in range(len(image_data)):
        bezier = osp.join(root_dir, bezier_data[i])
        img_path = osp.join(root_dir, image_data[i])

        A, B, C, D, _, given_img_path, proj2d = load_bezier_datamat(bezier)

        assert given_img_path == image_data[
            i], f".mat record {given_img_path} but now we have {image_data[i]}, dir {root_dir} in mat {bezier_data[i]}"

        new_img, imshow_origin = load_captured_image(img_path, proj2d)

        x_lst, y_lst = BezierCurve(A, B, C, D).get_discretized_point_lst()
        ax = plt.subplot(4, 4, i + 1)
        ax.imshow(new_img, origin=imshow_origin)

        ax.plot(x_lst, y_lst, color='red', linewidth=3)

    fig.savefig(output)
    plt.cla()
    plt.clf()
    plt.close('all')
    print(f"draw bezier curves to {output}")


def draw_bezier_batch(i):
    front_image_data, front_bezier_data, back_image_data, back_bezier_data = fetch_fabric_data(
        i)
    draw_bezier(i, front_image_data, front_bezier_data, back_image_data,
                back_bezier_data)


def get_index_and_name_from_fabric_subdir(path):
    deli = get_delimiter()
    id = int(path.split(deli)[-1])
    # print(f"path {path} id {id}")
    pkl_path = "D:\\Projects\\Subspace\\Archive\\full_data.pkl"
    import pickle as pkl
    with open(pkl_path, 'rb') as f:
        cont = pkl.load(f)
        id_lst = cont['id']
        label_lst = cont['label']
        # print(label_lst, len(label_lst))
        # print(id, label_lst[id])
        # exit()
        assert len(id_lst) == len(label_lst)
        label = None
        for _idx, id_form in enumerate(id_lst):
            if id_form == id:
                label = label_lst[_idx]
                break
    if label == None:
        # print(f"id lst {id_lst}")
        raise ValueError(f"fail to get the name for {path}")
    return id, label


if __name__ == "__main__":

    # all bezier points & imgs is located in this directory
    root_dir = "D:\\Projects\\弯曲测量数据"
    fabric_measure_dir_lst = get_fabric_subdirs(root_dir)
    with Pool(10) as p:
        p.map(draw_bezier_batch, fabric_measure_dir_lst)