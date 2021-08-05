import numpy as np


def eval_func(G, ds, dx, rho_g, s, l, theta_cur, theta_prev):
    sqrt_val = np.sqrt((ds / dx)**2 - 1)
    part1 = G * np.arctan(sqrt_val)
    # print(f"dx {dx} ds {ds} G {G} sqrt_val {sqrt_val} arctan {np.arctan(sqrt_val)} part1 {part1} ")
    part2 = -rho_g * (s - l) * dx * ds
    part3 = G * (2 * theta_cur - theta_prev)
    sum = part1 + part2 + part3
    assert np.isnan(part1) == False
    assert np.isnan(part2) == False
    assert np.isnan(part3) == False
    return sum


def eval_func_deriv(G, dx, ds, rho_g, s, l):

    return -G / np.sqrt(ds**2 - dx**2) - rho_g * (s - l) * ds


def solve_dx_newton(G, ds, s, rho_g, l, theta_cur, theta_prev):
    print(f"input ds = {ds}")
    # init guess
    x = ds * 0.5
    eps = 1e-9
    func = 1
    iter = 1
    while (np.abs(func) > eps):
        func = eval_func(G, ds, x, rho_g, s, l, theta_cur, theta_prev)
        dfdx = eval_func_deriv(G, x, ds, rho_g, s, l)
        x_incre = -1e-2 / dfdx * func
        print(
            f"Delta x = {x}, func = {func}, dfdx = {dfdx}, Delta x_incre = {x_incre}"
        )
        x = x + x_incre
        assert np.isnan(x) == False, f"x {x}"
        iter += 1

    print(f"iter {iter} func {func} Delta x {x}")
    return x


    # exit()
    # return x
def get_sign(val):
    if val > 0:
        return 1
    elif val < 0:
        return -1
    else:
        raise ValueError()


def solve_dx_bi(G, ds, s, rho_g, l, theta_cur, theta_prev, max_dx):
    x_left = 1e-4
    x_right = max_dx

    eps = 1e-10
    iter = 0
    max_iter = 100
    func_left_end = eval_func(G, ds, x_left, rho_g, s, l, theta_cur,
                              theta_prev)
    func_right_end = eval_func(G, ds, ds, rho_g, s, l, theta_cur, theta_prev)
    while iter < max_iter:

        dx = (x_left + x_right) / 2
        func = eval_func(G, ds, dx, rho_g, s, l, theta_cur, theta_prev)

        dKds = np.arctan(
            np.sqrt((ds / dx)**2 - 1)) + 2 * theta_cur - theta_prev
        dtheta_ds = (np.arctan(-np.sqrt(ds**2 - dx**2) / dx) - theta_cur) / ds
        if np.abs(func) < eps:
            print(
                f"[log] convergence: func {func}, dx = {dx} dKds = {dKds}, dtheta_ds = {dtheta_ds}"
            )
            break
        else:
            func_left = eval_func(G, ds, x_left, rho_g, s, l, theta_cur,
                                  theta_prev)
            func_right = eval_func(G, ds, x_right, rho_g, s, l, theta_cur,
                                   theta_prev)
            # print(
            #     f"[log] iter {iter} x_left = {x_left}, x_right = {x_right}, f(x_left) = {func_left}, f(x_right) = {func_right}"
            # )

            left_sign = get_sign(func_left)
            right_sign = get_sign(func_right)
            mid_sign = get_sign(func)
            assert left_sign != right_sign, f"{left_sign} != {right_sign} is not met"
            select_mode = None
            if mid_sign != left_sign:
                select_mode = 'left'
                x_right = dx
            elif mid_sign != right_sign:
                select_mode = 'right'
                x_left = dx
            # print(
            #     f"[log] sign dist: {left_sign} {mid_sign} {right_sign}, select {select_mode}"
            # )

        iter += 1
    if iter >= max_iter:
        print(
            f"[warn] max iter {iter} Delta x = {dx} func = {func}, func_left_end = {func_left_end}, func_right_end = {func_right_end}, dKds = {dKds}, dtheta_ds = {dtheta_ds}"
        )
    # print(f"---------solve dx done = {dx}-------------")
    return dx


def solve(G, rho_g, arc_length, theta_cur, theta_prev, samples=500):

    ds = arc_length / samples

    x_cur = 0.0
    y_cur = 0.0
    x_lst = [x_cur]
    y_lst = [y_cur]
    s = 0.0
    dx = ds
    for _ in range(samples):
        # 1. solve for dx
        # dx = solve_dx_newton(G, ds, s, rho_g, arc_length, theta_cur,
        #                      theta_prev)
        try:
            dx = solve_dx_bi(G, ds, s, rho_g, arc_length, theta_cur,
                             theta_prev, max_dx = dx)
        except Exception as e:
            print(f"get the exception {e}")
            break

        assert dx > 0, f"{dx}"
        # 2. get the dx and dy, get the new xy
        dy = -np.sqrt(ds**2 - dx**2)
        x_cur += dx
        y_cur += dy
        x_lst.append(x_cur)
        y_lst.append(y_cur)

        # 3. update the current arc length s
        s += ds

        # 4. update the theta_t and theta_{t-1}
        theta_prev = theta_cur
        theta_cur = np.arctan(dy / dx)
    return x_lst, y_lst


def test_deriv(G, rho_g, arc_length, theta_cur, theta_prev):
    ds = arc_length / 200
    dx = ds / 2
    s = arc_length / 2
    old_func = eval_func(G, ds, dx, rho_g, s, arc_length, theta_cur,
                         theta_prev)
    new_func = eval_func(G, ds, dx + 1e-5, rho_g, s, arc_length, theta_cur,
                         theta_prev)
    num_deriv = (new_func - old_func) / 1e-5
    ana_deriv = eval_func_deriv(G, dx, ds, rho_g, s, arc_length)
    print(f"num {num_deriv} ana {ana_deriv}")


if __name__ == "__main__":
    G = 268.3548439843612
    rhog = 117.28818181818183
    L = 11.086366382348894
    theta0 = -0.4615147244091352
    theta_minus1 = -0.43829467490532326


    x_lst, y_lst = solve(G, rhog, L, theta0, theta_minus1, samples=1500)
    import matplotlib.pyplot as plt
    plt.plot(x_lst, y_lst)
    plt.xlim(0, 2)
    plt.ylim(-2, 0)
    plt.show()
    # test_deriv(G, rhog, L, theta0, theta_minus1)

    # dx = ds
    # for i in range(10):
    #     func = eval_func(G, ds, dx, rho_g, s, l, theta_cur, theta_prev)
    #     print(f"Delta x = {dx}, func = {func}")
    #     dx += ds / 10