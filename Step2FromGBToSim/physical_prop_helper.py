import numpy as np


def get_density_g_inv_m2():
    '''
        get the density g.m^{-2}
    '''
    return None


def get_factor_matrix():
    factor_matrix = np.zeros([3, 3])

    factor_matrix[0, 0] = 0.216494632260111  #  数据来自数据点在Matlab上多元非线性拟合
    factor_matrix[
        0, 1] = 0.0227527139889231  #  代表一阶线性化后的，界面bending参数对对应方向bending强度贡献
    factor_matrix[0, 2] = 0.183236582216216

    factor_matrix[1, 0] = 0.0227527139889231
    factor_matrix[1, 1] = 0.216494632260111
    factor_matrix[1, 2] = 0.183236582216216

    factor_matrix[2, 0] = 0.091618291108108
    factor_matrix[2, 1] = 0.091618291108108
    factor_matrix[2, 2] = 0.239247346249034
    factor_matrix /= 50000.0 / 1.100  # !	理论系数alpha与模拟属性之间的校正

    # print(f"factor_matrix 1\n{factor_matrix}")

    test_value0 = factor_matrix.dot(np.array([1., 1., 1.]))
    # print(f"test value 0 {test_value0}")
    diag = np.array(
        [factor_matrix[0, 0], factor_matrix[1, 1], factor_matrix[2, 2]])
    # print(f"diag {diag}")

    factor_matrix[0, 0] = 0
    factor_matrix[1, 1] = 0
    factor_matrix[2, 2] = 0
    factor_matrix *= 0.1
    factor_matrix[0, 0] = diag[0]
    factor_matrix[1, 1] = diag[1]
    factor_matrix[2, 2] = diag[2]
    # print(f"factor_matrix 2 \n {factor_matrix}")

    test_value1 = factor_matrix.dot(np.array([1., 1., 1.]))
    # print(f"test value 1 {test_value1}")
    factor_matrix *= (test_value0[0] / test_value1[1])
    # print(f"factor_matrix 3 {factor_matrix}")

    return factor_matrix


def get_bending_length_m():
    '''
    get bending length [m]
    weft, warp, bias
    '''
    return None, None, None


def use_nonlinear(weft_length, warp_length):
    ml = max(weft_length, warp_length)
    ms = min(weft_length, warp_length)
    return (ml / ms) > 1.2


def get_bending_observed_stiffness(density, bending_length):
    stiff_factor = 1 / 8
    gravity = 9.8
    return density * gravity * (bending_length**3) * stiff_factor


def bias_clamp(weft, warp, bias):
    return np.clip(bias, min(weft, warp), max(weft, warp))


def get_GS_mat(factor_mat):
    StS = np.dot(factor_mat.T, factor_mat)
    return StS


def convert_bending_simtogui(value):
    bending_map_guitosim = [[0e0, 0.0], [1e2, 10.0], [1e3, 28.0], [3e3, 48.0],
                            [2e4, 65.0], [2e5, 83.0], [2e6, 100.0]]

    if value < bending_map_guitosim[0][0]:
        return bending_map_guitosim[0][1]
    for _idx in range(len(bending_map_guitosim) - 1):
        sim_st, gui_st = bending_map_guitosim[_idx]
        sim_ed, gui_ed = bending_map_guitosim[_idx + 1]
        if sim_st <= value <= sim_ed:
            return gui_st + (value - sim_st) / (sim_ed - sim_st) * (gui_ed -
                                                                    gui_st)
    if value > bending_map_guitosim[-1][0]:
        return bending_map_guitosim[-1][1]