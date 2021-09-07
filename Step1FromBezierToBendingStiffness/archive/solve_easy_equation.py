import numpy as np
from matplotlib import pyplot as plt


def get_analytic_comp(x_st, x_ed, steps):
    x = np.linspace(x_st, x_ed, steps)
    y = np.exp(x) + x + 2
    return x, y


def get_analytic_ex(x_st, x_ed, steps):
    x = np.linspace(x_st, x_ed, steps)
    y = np.exp(x)
    return x, y


def forward_euler(x_t, w_lst, dx, f_list):
    x_next = x_t + dx
    y_next_lst = []
    for i in range(len(w_lst)):
        y_next = dx * f_list[i](x_t, *w_lst) + w_lst[i]
        y_next_lst.append(y_next)
    # y_next = dx * y_func(x_t, u_tas, y_t) + u_t
    if len(y_next_lst) == 1:
        return x_next, y_next_lst[0]
    elif len(y_next_lst) == 2:
        return x_next, y_next_lst[0], y_next_lst[1]
    else:
        raise ValueError()


def forward_RK4(x_t, w_lst, dx, f_lst):
    t = x_t
    h = dx
    num_of_vars = len(w_lst)
    assert len(f_lst) == num_of_vars

    k1_lst = []
    k2_lst = []
    k3_lst = []
    k4_lst = []
    # 1. calculate k1 for all varialbes
    for j in range(num_of_vars):
        k1j = h * f_lst[j](t, *w_lst)
        k1_lst.append(k1j)
    # 2. calculate k2 for all varialbes
    for j in range(num_of_vars):
        k2j = h * f_lst[j](t + h / 2,
                           *(list(np.array(w_lst) + 0.5 * np.array(k1_lst))))
        k2_lst.append(k2j)

    # 3. calculate k3 for all varialbes
    for j in range(num_of_vars):
        k3j = h * f_lst[j](t + h / 2,
                           *(list(np.array(w_lst) + 0.5 * np.array(k2_lst))))
        k3_lst.append(k3j)
    # 4. calculate k4 for all varialbes
    for j in range(num_of_vars):
        k4j = h * f_lst[j](t + h,
                           *(list(np.array(w_lst) + 1 * np.array(k3_lst))))
        k4_lst.append(k4j)

    new_w_lst = list(
        np.array(w_lst) + (np.array(k1_lst) + 2 * np.array(k2_lst) +
                           2 * np.array(k3_lst) + np.array(k4_lst)) / 6)
    x_next = t + h

    if len(new_w_lst) == 1:
        return x_next, new_w_lst[0]
    elif len(new_w_lst) == 2:
        return x_next, new_w_lst[0], new_w_lst[1]
    else:
        raise ValueError()


if __name__ == "__main__":
    '''
        v = d/dx = f(x, u0, u1)
        dv/dx = e^x
        y(0) = 3

        y = e^x + x + 2

        y(0) = 1 + 2  = 3

        v = e^x + 1
        v(0) = 2
    '''
    x_st = 0
    x_ed = 3
    steps = 10
    dx = (x_ed - x_st) / (steps - 1)
    def y_func(x, u, y):
        return u

    def u_func(x, u, y):
        return np.exp(x) + 1

    xt = 0
    ut = 2
    yt = 3
    x_lst_euler, y_lst_euler = [], []
    for i in range(steps):
        x_lst_euler.append(xt)
        y_lst_euler.append(yt)

        xt, ut, yt = forward_euler(xt, [ut, yt], dx, [y_func, u_func])
        # xt, ut, yt = forward_RK4(xt, [ut, yt], dx, [y_func, u_func])
    xt = 0
    ut = 2
    yt = 3
    x_lst_rk4, y_lst_rk4 = [], []
    for i in range(steps):
        x_lst_rk4.append(xt)
        y_lst_rk4.append(yt)

        # xt, ut, yt = forward_euler(xt, [ut, yt], dx, [y_func, u_func])
        xt, ut, yt = forward_RK4(xt, [ut, yt], dx, [y_func, u_func])

    ana_x, ana_y = get_analytic_comp(x_st, x_ed, steps)
    plt.subplot(1, 2, 1)
    plt.plot(ana_x, ana_y, label='ana')
    plt.plot(x_lst_euler, y_lst_euler, label='num euler')
    plt.plot(x_lst_rk4, y_lst_rk4, label='num rk4')
    plt.legend()
    plt.subplot(1, 2, 2)
    plt.plot(ana_y - y_lst_euler, label='euler diff')
    plt.plot(ana_y - y_lst_rk4, label='rk4 diff')
    plt.legend()
    plt.show()
# if __name__ == "__main__":
#     '''
#         dydx = e^x
#     '''
#     x_st = 0
#     x_ed = 3
#     xt = x_st
#     yt = 1
#     steps = 100
#     dx = (x_ed - x_st) / (steps - 1)

#     x_lst_euler, y_lst_euler = [], []
#     x_lst_rk4, y_lst_rk4 = [], []

#     def y_func(x, y):
#         return np.exp(x)

#     for i in range(steps):
#         x_lst_euler.append(xt)
#         y_lst_euler.append(yt)

#         xt, yt = forward_euler(xt, [yt], dx, [y_func])

#     # ---------- restart ----------
#     xt = x_st
#     yt = 1
#     for i in range(steps):
#         x_lst_rk4.append(xt)
#         y_lst_rk4.append(yt)

#         xt, yt = forward_RK4(xt, [yt], dx, [y_func])

#     ana_x, ana_y = get_analytic_ex(x_st, x_ed, steps)
#     plt.subplot(1, 2, 1)
#     plt.plot(ana_x, ana_y, label='ana')
#     plt.plot(x_lst_euler, y_lst_euler, label='euler num')
#     plt.plot(x_lst_rk4, y_lst_rk4, label='RK4 num')
#     plt.legend()
#     plt.subplot(1, 2, 2)
#     plt.plot(ana_y - y_lst_euler, label='euler diff')
#     plt.plot(ana_y - y_lst_rk4, label='rk4 diff')
#     plt.legend()
#     plt.show()