import pickle as pkl
import numpy as np


def get_bending_data():
    data_file = "full_data.pkl"
    with open(data_file, 'rb') as f:
        data = pkl.load(f)
    
    # anisotropic_bending = observedBendingWeft
    full_bending_lst = data["anisotropic_bending"]
    # 1. simplify the data: do averge over the two sides
    # 2. delete all ampty data
    valid_bending_lst = []
    new_id = 0
    for _idx, i in enumerate(full_bending_lst):
        # i.shape = 9x3
        # 9 row: 0, 22.5, ... 180 degs
        # 3 col: angle degree, one side, another side
        if i.shape == (9, 3):
            bending0_oneside = i[0][1]
            bending90_oneside = i[2][1]
            bending180_oneside = i[4][1]
            valid_bending_lst.append(
                (new_id, _idx, data["label"][_idx],
                 [bending0_oneside, bending90_oneside, bending180_oneside]))
            new_id += 1
    return valid_bending_lst

from gauss_seidel import calc_bending_params

if __name__ == "__main__":
    # 1. the data 
    data_lst = get_bending_data()
    '''
        the call map
        1. Convert2Fab.m
        2. GenBendingData.m
        3. Compute180ByFittingCurve.m
        4. FitBendingCurve.m (FitBendingCurve (A,B,C,D) 
        5. fitnlm : fit s and x and model function

        s ? the length of the arc
        x ? cartesian x coords
        model function ? interpolation function

        please check the comment in FitBendingCurve.m
    
    '''
    # full data, anistropic bending = alpha / (rho * g)
    # alpha unit ? 
    # rho unit ?
    # g unit ?
    print(f"data lst {data_lst}")
    # for i in data_lst:
    #     bending = calc_bending_params(*i[3])
    #     print(f"obs {i[3]}cm sim {bending}")

# 1. read the image data, click the point, output the bezier curve: CurveAcquisition2.mlapp

# 2. calculate the only one stiffness parameter from this curve (is it possible?): ViewKmFitting.m
# 3. convert this stiffness parameter to the style3d unit:
# 4. run simulation and extract the cruve, do comparision