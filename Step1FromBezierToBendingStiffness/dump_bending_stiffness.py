'''
calculate all bending stiffness, write down the result to a info dict, save
'''
import os.path as osp
from load_bending_bezier_data import get_fabric_subdirs, fetch_fabric_data, get_index_and_name_from_fabric_subdir
from calculate_bending_stiffness_from_bezier import get_linear_density_for_specimen, get_specimen_width_cm, calculate_bending_stiffness_from_bezier

if __name__ == "__main__":
    root_dir = "D:\\Projects\\弯曲测量数据"
    # info_dict = {
    linear_density_lst = []
    density_lst = []
    idx_lst = []
    name_lst = []
    angles_lst = []
    front_stiffness_lst = []
    back_stiffness_lst = []

    num_of_angles = 8
    for fabric_subdir in get_fabric_subdirs(root_dir):
        linear_density = get_linear_density_for_specimen(fabric_subdir)
        density = 1 / (1e-2 * get_specimen_width_cm()) * linear_density

        fabric_id, name = get_index_and_name_from_fabric_subdir(fabric_subdir)
        front_image_data_lst, front_bezier_data_lst, back_image_data_lst, back_bezier_data_lst = fetch_fabric_data(
            fabric_subdir)

        front_angles, back_angles = [], []
        for _idx in range(num_of_angles):
            front_image_data = front_image_data_lst[_idx]
            front_bezier_data = front_bezier_data_lst[_idx]
            cur_root_dir = osp.join(root_dir, str(fabric_id))
            front_k = calculate_bending_stiffness_from_bezier(
                cur_root_dir, front_bezier_data, front_image_data)
            # print(f"front {_idx} {front_k}")
            front_angles.append(front_k)

            back_image_data = back_image_data_lst[_idx]
            back_bezier_data = back_bezier_data_lst[_idx]
            back_k = calculate_bending_stiffness_from_bezier(
                cur_root_dir, back_bezier_data, back_image_data)
            # print(f"back {_idx} {back_k}")
            back_angles.append(back_k)
        # print(f"front {front_angles}")
        # print(f"back {back_angles}")
        # exit()
        # push into the list

        print(fabric_id, name, linear_density, density)

        linear_density_lst.append(linear_density)
        density_lst.append(density)
        idx_lst.append(fabric_id)
        name_lst.append(name)
        front_stiffness_lst.append(front_angles)
        back_stiffness_lst.append(back_angles)
    unit = "N.m^2"
    angles = [i * 22.5 for i in range(num_of_angles)]
    data_dict = {
        "name_lst": name_lst,
        "idx_lst": idx_lst,
        "density_lst": density_lst,
        "linear_density_lst": linear_density_lst,
        "front_bending_stiffness_lst": front_stiffness_lst,
        "back_bending_stiffness_lst": back_stiffness_lst,
        "angles": angles,
        "unit": unit
    }
    output = "output/bending_stiffness_data.pkl"
    import pickle as pkl
    with open(output, 'wb') as f:
        pkl.dump(data_dict, f)
        print(f"save to {output}")