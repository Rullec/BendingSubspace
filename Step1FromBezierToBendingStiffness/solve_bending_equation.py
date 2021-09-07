import numpy as np
from matplotlib import pyplot as plt


def normalize_angle(theta):
    while theta > np.pi:
        theta -= np.pi * 2
    while theta < -np.pi:
        theta += np.pi * 2
    return theta


def calculate_angle_error(theta_lst):
    err_angle = 0
    for i in theta_lst:
        if i > 0:
            err_angle += i
        elif i < -np.pi / 2:
            err_angle += -np.pi / 2 - i
    return err_angle


def solve_guess_IVP_normalized(beta, theta0, x0, y0, t):
    M_lst = []
    theta_lst = []
    x_lst = []
    y_lst = []

    steps = 1000
    h = 1 / (steps - 1)

    x_cur = x0
    y_cur = y0
    s_cur = 0
    theta_cur = theta0
    M_cur = t

    f1 = lambda s, theta, M: beta * M
    f2 = lambda s, theta, M: (s - 1) * np.cos(theta)

    for i in range(steps):
        M_lst.append(M_cur)
        theta_lst.append(theta_cur)
        x_lst.append(x_cur)
        y_lst.append(y_cur)

        # ----------- explicit euler ----------
        theta_new = theta_cur + beta * M_cur * h
        M_new = M_cur + h * (s_cur - 1) * np.cos(theta_cur)

        theta_cur = theta_new
        M_cur = M_new

        # ------------ RK4 ------------
        # k1_f1 = h * f1(s_cur, theta_cur, M_cur)
        # k1_f2 = h * f2(s_cur, theta_cur, M_cur)

        # k2_f1 = h * f1(s_cur + h / 2, theta_cur + 0.5 * k1_f1,
        #                M_cur + 0.5 * k1_f2)
        # k2_f2 = h * f2(s_cur + h / 2, theta_cur + 0.5 * k1_f1,
        #                M_cur + 0.5 * k1_f2)

        # k3_f1 = h * f1(s_cur + h / 2, theta_cur + 0.5 * k2_f1,
        #                M_cur + 0.5 * k2_f2)
        # k3_f2 = h * f2(s_cur + h / 2, theta_cur + 0.5 * k2_f1,
        #                M_cur + 0.5 * k2_f2)

        # k4_f1 = h * f1(s_cur + h, theta_cur + k3_f1, M_cur + k3_f2)
        # k4_f2 = h * f2(s_cur + h, theta_cur + k3_f1, M_cur + k3_f2)

        # theta_cur = theta_cur + (k1_f1 + 2 * k2_f1 + 2 * k3_f1 + k4_f1) / 6
        # M_cur = M_cur + (k1_f2 + 2 * k2_f2 + 2 * k3_f2 + k4_f2) / 6

        s_cur += h
        x_cur += h * np.cos(theta_cur)
        y_cur += h * np.sin(theta_cur)

    err_t = np.abs(h * np.sum(x_lst) - t)
    err_angle = calculate_angle_error(theta_lst) * h
    err = err_t + err_angle
    # print(f"err angle {err_angle}, err t {err_t}, total err {err}")
    return err, x_lst, y_lst


def shoot_solve_normalzed(beta, theta0, t0, stepsize, silent=True):
    t1 = 0.99 * t0
    eps = 1e-3
    # 1. select t0 and t1
    x0 = 0
    y0 = 0

    t_prev = t0
    t_cur = t1
    err_prev, x_, y_ = solve_guess_IVP_normalized(beta, theta0, x0, y0, t_prev)
    # plt.plot(x_, y_)
    # plt.show()
    err_cur, x__, y__ = solve_guess_IVP_normalized(beta, theta0, x0, y0, t_cur)

    if np.abs(err_prev) < eps:
        return x_, y_
    elif np.abs(err_cur) < eps:
        return x__, y__

    # 2. calculate the f(t0) and f(t1)
    max_shooting = 10000
    for k in range(2, max_shooting + 1):
        t_cur = t_cur - stepsize * err_cur * (t_cur - t_prev) / (err_cur -
                                                                 err_prev)
        err_cur, x_, y_ = solve_guess_IVP_normalized(beta, theta0, x0, y0,
                                                     t_cur)
        if silent is False:
            if k % 10 == 0 or err_cur < eps:
                print(f"[debug] iter {k} t {t_cur} t0 {t0}, error {err_cur}")
        if err_cur < eps:
            break
    if k == max_shooting:
        return None, None
    else:
        return x_, y_


# def shooting_solve(G,
#                    rho_g,
#                    L,
#                    theta0,
#                    x0,
#                    y0,
#                    t0=500,
#                    stepsize=1e-2,
#                    max_shooting=200):
#     eps = 1
#     t1 = 0.99 * t0
#     # 1. select t0 and t1

#     t_prev = t0
#     t_cur = t1
#     f_prev, x_, y_ = solve_guess_IVP(G, rho_g, L, theta0, x0, y0, t_prev)
#     # plt.plot(x_, y_)
#     # plt.show()
#     f_cur, x__, y__ = solve_guess_IVP(G, rho_g, L, theta0, x0, y0, t_cur)

#     if f_prev < eps:
#         return x_, y_
#     elif f_cur < eps:
#         return x__, y__

#     # 2. calculate the f(t0) and f(t1)
#     for k in range(2, max_shooting + 1):
#         t_cur = t_cur - stepsize * f_cur * (t_cur - t_prev) / (f_cur - f_prev)
#         f_cur, x_, y_ = solve_guess_IVP(G, rho_g, L, theta0, x0, y0, t_cur)
#         if k % 50 == 0 or f_cur < eps:
#             print(f"[debug] iter {k} t {t_cur}, error {f_cur}")
#         if f_cur < eps:
#             break
#     if k == max_shooting:
#         return None, None
#     else:
#         return x_, y_

if __name__ == "__main__":
    G = 268.3548439843612
    rhog = 117.28818181818183
    L = 4.1086366382348894
    theta0 = -0.4615147244091352

    beta = -1 * rhog * (L**3) / G
    # x_lst, y_lst = shooting_solve(G, rhog, L, theta0, 0, 0)
    x_lst, y_lst = shoot_solve_normalzed(beta, theta0)

    width = max(max(x_lst) - min(x_lst), max(y_lst) - min(y_lst))

    # print(f"x_lst {x_lst}")
    # print(f"y_lst {y_lst}")
    plt.plot(x_lst, y_lst)
    plt.xlim(min(x_lst), min(x_lst) + width)
    plt.ylim(min(y_lst), min(y_lst) + width)
    # plt.title(f"G {G:.3f}, rhog {rhog:.3f}, L {L:.3f}, theta {theta0:.3f}")
    plt.title(f"beta {beta:.3f}")
    plt.show()