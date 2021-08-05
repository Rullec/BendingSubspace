import numpy as np


def get_factormat():
    factor_mat = np.array([[
        0.216494632260111,
        0.0227527139889231,
        0.183236582216216,
    ], [
        0.0227527139889231,
        0.216494632260111,
        0.183236582216216,
    ], [
        0.091618291108108,
        0.091618291108108,
        0.239247346249034,
    ]])

    factor_mat /= (50000 / 1.1)

    test_value0 = np.dot(factor_mat, np.ones(3))

    diag = factor_mat.diagonal()
    # print(diag)

    for i in range(3):
        factor_mat[i, i] = 0
    # print(factor_mat)
    factor_mat *= 0.1

    for i in range(3):
        factor_mat[i, i] = diag[i]

    test_value1 = np.dot(factor_mat, np.ones(3))
    factor_mat *= test_value0[0] / test_value1[0]
    return factor_mat


def calc_density(size0, size1, weight):
    rect_area = size0 * size1 * 1e-6
    density = weight / rect_area
    return density


def get_observed_bending_data(weft_length, warp_length, bias_length, density):
    weft_length *= 1e-3
    warp_length *= 1e-3
    bias_length *= 1e-3
    g = 9.8
    stiffFactor = 1.0 / 8
    ml = max(weft_length, warp_length)
    ms = min(weft_length, warp_length)
    use_nonlinear = ml / ms > 1.2

    observed_bending_weft = density * g * (weft_length**3) * stiffFactor
    observed_bending_warp = density * g * (warp_length**3) * stiffFactor
    observed_bending_bias = density * g * (bias_length**3) * stiffFactor

    def clamp(val, lim0, lim1):
        if lim0 > lim1:
            tmp = lim1
            lim1 = lim0
            lim0 = tmp
        val = np.clip(val, lim0, lim1)
        return val

    # do clamp
    observed_bending_bias = clamp(observed_bending_bias, observed_bending_warp,
                                  observed_bending_weft)
    return observed_bending_weft, observed_bending_warp, observed_bending_bias, use_nonlinear


def nonlinear_gauss(out, mat, rhs):
    for iter in range(500):
        for i in range(2):
            top = rhs[i]
            for j in range(3):
                if i != j:
                    top -= mat[i][j] * out[j]
            out[i] = top / mat[i][i]
            out[i] = max(out[i], 0.)

        i = 2
        top = rhs[i]
        mi = min(out[1], out[0])
        top -= mat[i][0] * 2 * mi
        out[i] = top / mat[i][i]
        out[i] = max(out[i], 0.)
    return out


def linear_gauss(out, mat, rhs):
    for iter in range(500):
        for i in range(3):
            top = rhs[i]
            for j in range(3):
                if i != j:
                    top -= mat[i][j] * out[j]

            out[i] = top / mat[i][i]
            out[i] = max(out[i], 0.)

        for i in range(2, -1, -1):
            top = rhs[i]
            for j in range(3):
                if i != j:
                    top -= mat[i][j] * out[j]
            out[i] = top / mat[i][i]
            out[i] = max(out[i], 0.)
    return out


def calculate_sim_params_from_obs(obs_weft, obs_warp, obs_bias, factor_mat,
                                  use_nonlinear: bool):
    rhs = np.array([obs_weft, obs_warp, obs_bias])
    sts = np.matmul(factor_mat.T, factor_mat)
    strhs = np.dot(factor_mat.T, rhs)

    bending_stiff = rhs
    b = strhs

    mat = sts

    if use_nonlinear:
        bending_stiff = nonlinear_gauss(bending_stiff, mat, b)
    else:
        bending_stiff = linear_gauss(bending_stiff, mat, b)

    return bending_stiff


def calc_bending_params(observed_bending_weft, observed_bending_warp, observed_bending_bias):
    factor_mat = get_factormat()
    # density = calc_density(size0, size1, weight)
    bending_params = calculate_sim_params_from_obs(observed_bending_weft,
                                                   observed_bending_warp,
                                                   observed_bending_bias,
                                                   factor_mat, use_nonlinear = True)
    return bending_params


if __name__ == "__main__":
    pass