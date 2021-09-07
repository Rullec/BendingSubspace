import numpy as np


def f1(s, x, w1, w2):
    return w1


def f2(s, x, w1, w2):
    return w2


def f3(rho_g, G, L, s, x, w1, w2):
    assert np.abs(w1) < 1, f"{w1}"
    # return rho_g / G * (L - s) * w1 * np.sqrt(1 - w1**2) + 2 * w2**2 / w1 - 2 * (
    #     w2 / w1)**2
    return rho_g / G * (L - s) * w1 * np.sqrt(1 - w1**2) - (w2 ** 2) * w1 / (1 - w1 ** 2)


def solve_euler(rho_g, G, L, x_cur, ds, s_cur, w1, w2):
    assert np.abs(w1) < 1, f"abs w1 {np.abs(w1)} < 1 "
    assert w1 > 0, f"w1 {w1}"
    # assert w2 < 0, f"w2 {w2}"
    dx = ds * w1
    print(f"s {s_cur} w1 {w1} w2 {w2} ds {ds} dx {dx}")
    x_new = ds * w1 + x_cur
    print(f"x new {x_new}")
    w1_new = w2 * ds + w1
    print(f"w1 new {w1_new}")

    # dw2ds = (rho_g / G * (L - s_cur) * w1 * np.sqrt(1 - w1**2) + 2 *
    #          (w2**2) / w1 - 2 * (w2 / w1)**2)
    dw2ds = f3(rho_g, G, L, s_cur, w1, w2)
    assert dw2ds < 0, f"dw2ds = {dw2ds}"
    w2_new = ds * dw2ds + w2
    print(f"w2 new {w2_new}")
    s_new = s_cur + ds
    return s_new, x_new, w1_new, w2_new


def solve_rk4(rho_g, G, L, x_cur, ds, s_cur, w1, w2):
    # t = s_cur
    print(f"w1 {w1} w2 {w2} cur s {s_cur} < L {L}")
    h = ds
    w_lst = [x_cur, w1, w2]

    k1_lst = [
        h * f1(s_cur, x_cur, w1, w2),
        h * f2(s_cur, x_cur, w1, w2),
        h * f3(rho_g, G, L, s_cur, x_cur, w1, w2),
    ]
    # print(f"k1 {k1_lst}")
    k2_lst = [
        h * f1(s_cur + h / 2, *list(np.array(w_lst) + 0.5 * np.array(k1_lst))),
        h * f2(s_cur + h / 2, *list(np.array(w_lst) + 0.5 * np.array(k1_lst))),
        h * f3(rho_g, G, L, s_cur + h / 2,
               *list(np.array(w_lst) + 0.5 * np.array(k1_lst))),
    ]

    k3_lst = [
        h * f1(s_cur + h / 2, *list(np.array(w_lst) + 0.5 * np.array(k2_lst))),
        h * f2(s_cur + h / 2, *list(np.array(w_lst) + 0.5 * np.array(k2_lst))),
        h * f3(rho_g, G, L, s_cur + h / 2,
               *list(np.array(w_lst) + 0.5 * np.array(k2_lst))),
    ]
    k4_lst = [
        h * f1(s_cur + h, *list(np.array(w_lst) + np.array(k3_lst))),
        h * f2(s_cur + h, *list(np.array(w_lst) + np.array(k3_lst))),
        h *
        f3(rho_g, G, L, s_cur + h, *list(np.array(w_lst) + np.array(k3_lst))),
    ]

    new_w_lst = list(
        np.array(w_lst) + (np.array(k1_lst) + 2 * np.array(k2_lst) +
                           2 * np.array(k3_lst) + np.array(k4_lst)) / 6)
    s_next = s_cur + h
    return s_next, new_w_lst[0], new_w_lst[1], new_w_lst[2]


def solve_new(rho_g, G, L, w1, w2):
    '''
        solve the 3 order ODE

        y^{(3)}(x) = rho_g / G ( L - x) y' \sqrt{1 - (y')^2} + 2 (y'')^2 / y' - 2 (y'' / y')^2
    '''
    steps = 1000
    ds = L / (steps - 1)

    x_st = 0
    y_st = 0
    s_st = 0

    x_cur = x_st
    y_cur = y_st
    s_cur = s_st
    x_lst, y_lst = [], []
    for step_id in range(steps):
        try:
            print(f"-----iter {step_id}----")
            x_lst.append(x_cur)
            y_lst.append(y_cur)

            # s_cur, x_new, w1, w2 = solve_euler(rho_g, G, L, x_cur, ds, s_cur,
            #                                    w1, w2)
            s_cur, x_new, w1, w2 = solve_rk4(rho_g, G, L, x_cur, ds, s_cur,
                                                w1, w2)
            assert np.isnan(x_new) == False, x_new
            assert np.isnan(w1) == False, w1
            assert np.isnan(w2) == False, w2
            dy = -np.sqrt(ds**2 - (x_new - x_cur)**2)
            y_cur = y_cur + dy
            x_cur = x_new
        except Exception as e:
            print(e)
            break
    return x_lst, y_lst


if __name__ == "__main__":
    G = 268.3548439843612
    rhog = 117.28818181818183
    L = 11.086366382348894
    theta0 = -0.4615147244091352
    theta_minus1 = -0.43829467490532326

    ds = 1e-2
    w1 = np.abs(np.cos(theta0))
    K = (theta0 - theta_minus1) / ds
    # K = 75
    w2 = -K * np.sin(theta0)

    x_lst, y_lst = solve_new(rhog, G, L, w1, w2)
    from matplotlib import pyplot as plt
    plt.plot(x_lst, y_lst)

    box = max(max(np.abs(x_lst)), max(np.abs(y_lst)))
    plt.xlim(-box, box)
    plt.ylim(-box, box)
    plt.show()