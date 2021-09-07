import numpy as np


def bending_nonlinear_gauss_seidel(mat, rhs):
    assert type(mat) == np.ndarray, type(mat)
    assert type(rhs) == np.ndarray, type(rhs)
    assert mat.shape == (3, 3)
    assert rhs.shape == (3, )

    out = np.zeros(3)
    for iter in range(500):
        for i in range(2):
            top = rhs[i]
            for j in range(3):
                if i != j:
                    top -= mat[i, j] * out[j]

            out[i] = top / mat[i, i]
            out[i] = max(out[i], 0)
        i = 2
        top = rhs[i]
        mi = min(out[1], out[0])
        top -= mat[i, 0] * 2 * mi
        out[i] = top / mat[i, i]
        out[i] = max(out[i], 0)
    return out


def bending_linear_gauss_seidel(mat, rhs):
    assert type(mat) == np.ndarray
    assert type(rhs) == np.ndarray
    assert mat.shape == (3, 3)
    assert rhs.shape == (3, )

    out = np.zeros(3)
    for iter in range(500):
        for i in range(3):
            top = rhs[i]
            for j in range(3):
                if i != j:
                    top -= mat[i, j] * out[j]
            out[i] = top / mat[i, i]
            out[i] = max(out[i], 0)
        for i in reversed(range(3)):
            top = rhs[i]
            for j in range(3):
                if i != j:
                    top -= mat[i, j] * out[j]
            out[i] = top / mat[i, i]
            out[i] = max(out[i], 0)
    return out