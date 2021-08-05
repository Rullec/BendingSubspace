import numpy as np

def parse_single_cloth(data):
    # strucutred type
    # strucuted array
    data_keys =  list(data.dtype.fields.keys())
    data_value = [i for i in data.tolist()]
    new_data = {}
    for i in range(len(data_keys)):
        new_data[data_keys[i]] = data_value[i]
    return new_data

def parse_matlab_data(mat_file):
    from scipy.io import loadmat
    # 1. load .mat files
    mat_data = loadmat(mat_file, squeeze_me = True)
    mat_data_lst = mat_data["full_data"]

    # 2. iterate over matlab data cells, get the value dict
    num_of_data = len(mat_data_lst)
    dict_data_lst = []
    for i in range(num_of_data):
        data = parse_single_cloth(mat_data_lst[i])
        dict_data_lst.append(data)
    
    # 3. re arrange the data list
    dict_lst_data = {}
    data_keys = list(dict_data_lst[0].keys())
    for i in data_keys:
        dict_lst_data[i] = []
    for i in dict_data_lst:
        for j in i:
            dict_lst_data[j].append(i[j])
    return dict_lst_data


    

import pickle
import os
import shutil
if __name__ == "__main__":
    mat_file = "full_data.mat"
    output_file = "full_data.pkl"
    data = parse_matlab_data(mat_file)

    if os.path.exists(output_file) == True:
        os.remove(output_file)

    with open(output_file, 'wb') as f:
        pickle.dump(data, f)
