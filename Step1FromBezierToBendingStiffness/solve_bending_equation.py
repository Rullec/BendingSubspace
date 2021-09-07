import numpy as np
from matplotlib import pyplot as plt


def solve_guess_IVP(G, rho_g, L, theta0, x0, y0, t):
    M_lst = []
    theta_lst = []
    x_lst = []
    y_lst = []

    h = 1e-2
    steps = int(L / h)
    # h = L / (steps - 1)

    x_cur = x0
    y_cur = y0
    s_cur = 0
    theta_cur = theta0
    M_cur = t

    f1 = lambda s, theta, M: -M / G
    f2 = lambda s, theta, M: rho_g * (s - L) * np.cos(theta)

    for i in range(steps):
        M_lst.append(M_cur)
        theta_lst.append(theta_cur)
        x_lst.append(x_cur)
        y_lst.append(y_cur)

        # ----------- explicit euler ----------
        theta_new = theta_cur - M_cur / G * h
        M_new = M_cur + h * rho_g * (s_cur - L) * np.cos(theta_cur)

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

    M_match_err = -t
    for i in x_lst:
        M_match_err += i * rho_g * h

    angle_range_err = 0
    for i in theta_lst:
        while i > np.pi:
            i -= 2 * np.pi

        while i < -np.pi:
            i += 2 * np.pi

        if i > 0:
            angle_range_err += i
        if i < -np.pi / 2:
            angle_range_err += -np.pi / 2 - i
    error = np.abs(M_match_err) + np.abs(angle_range_err)
    return error, x_lst, y_lst


def shooting_solve(G,
                   rho_g,
                   L,
                   theta0,
                   x0,
                   y0,
                   t0=500,
                   stepsize=1e-2,
                   max_shooting=200):
    eps = 1
    t1 = 0.99 * t0
    # 1. select t0 and t1

    t_prev = t0
    t_cur = t1
    f_prev, x_, y_ = solve_guess_IVP(G, rho_g, L, theta0, x0, y0, t_prev)
    # plt.plot(x_, y_)
    # plt.show()
    f_cur, x__, y__ = solve_guess_IVP(G, rho_g, L, theta0, x0, y0, t_cur)

    if f_prev < eps:
        return x_, y_
    elif f_cur < eps:
        return x__, y__

    # 2. calculate the f(t0) and f(t1)
    for k in range(2, max_shooting + 1):
        t_cur = t_cur - stepsize * f_cur * (t_cur - t_prev) / (f_cur - f_prev)
        f_cur, x_, y_ = solve_guess_IVP(G, rho_g, L, theta0, x0, y0, t_cur)
        if k % 50 == 0 or f_cur < eps:
            print(f"[debug] iter {k} t {t_cur}, error {f_cur}")
        if f_cur < eps:
            break
    if k == max_shooting:
        return None, None
    else:
        return x_, y_


if __name__ == "__main__":
    G = 268.3548439843612
    rhog = 117.28818181818183
    L = 6.1086366382348894
    theta0 = -0.4615147244091352

    x_lst, y_lst = shooting_solve(G, rhog, L, theta0, 0, 0)

    width = max(max(x_lst) - min(x_lst), max(y_lst) - min(y_lst))

    # print(f"x_lst {x_lst}")
    # print(f"y_lst {y_lst}")
    plt.plot(x_lst, y_lst)
    plt.xlim(min(x_lst), min(x_lst) + width)
    plt.ylim(min(y_lst), min(y_lst) + width)
    plt.title(f"G {G:.3f}, rhog {rhog:.3f}, L {L:.3f}, theta {theta0:.3f}")
    plt.show()