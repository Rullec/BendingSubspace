import numpy as np
from matplotlib import pyplot as plt


def factorial(n):
    assert n >= 0
    if n == 0:
        return 1
    val = 1
    for i in range(1, n + 1):
        val *= i
    return val


def Com(n, k):
    '''
    C^n_k = n! / (k! * (n-k)!)
    '''
    assert n >= 1
    assert k >= 0 and k <= n
    return int(factorial(n) / (factorial(k) * factorial(n - k)))


class BezierCurve(object):
    def __init__(self, *args, n=200):
        '''
        args: Given many bezier points
        n: sampling number over the arc length
        '''
        for i in args:
            assert i.size == 2, f"{len(i)}, {i}"

        self.num_of_samples = n
        self.bezier_points = args
        assert len(self.bezier_points) == 4
        self.order = len(self.bezier_points) - 1
        self.u = np.expand_dims(np.linspace(0, 1, num=self.num_of_samples),
                                axis=0)
        self.one_minus_u = 1 - self.u

        # calculate the points' position on the bezier curve
        self.x_lst, self.y_lst = self.__calc_discrete_point()
        # self.__cut_from_the_remotest_point()
        self.curvaute_lst = self.__calc_curvature_lst()
        # self.__cut_to_the_end_of_zero_curvature()
        self.__cur_from_the_biggest_curvature()
        self.ds_lst = None
        self.total_arc_length = None
        self.dx_lst = None
        self.dy_lst = None
        self.dKdx_lst = None
        self.theta_lst = None

    # def __cut_to_the_end_of_zero_curvature(self):
    #     cut_idx = None
    #     for i in range(1, len(self.curvaute_lst)):
    #         if self.curvaute_lst[i] > self.curvaute_lst[
    #                 i - 1] and self.curvaute_lst[i] < 1:
    #             cut_idx = i
    #             break
    #     if cut_idx is not None:
    #         self.x_lst = self.x_lst[:cut_idx]
    #         self.y_lst = self.y_lst[:cut_idx]
    #         self.u = self.u[:, :cut_idx]
    #         self.one_minus_u = self.one_minus_u[:, :cut_idx]
    #         self.curvaute_lst = self.curvaute_lst[:cut_idx]
    #         self.num_of_samples = len(self.x_lst)

    def __cur_from_the_biggest_curvature(self):
        cut_idx = int(np.argmax(self.curvaute_lst) * 1.5)
        self.x_lst = self.x_lst[cut_idx:]
        self.y_lst = self.y_lst[cut_idx:]
        self.u = self.u[:, cut_idx:]
        self.one_minus_u = self.one_minus_u[:, cut_idx:]
        self.curvaute_lst = self.curvaute_lst[cut_idx:]
        self.num_of_samples = len(self.x_lst)

    # def __cut_from_the_remotest_point(self):
    #     # the curve is supported by a extended platform
    #     # so we must cut the unfree part
    #     highest_idx = np.argmax(self.y_lst)
    #     longest_idx = np.argmax(self.x_lst)
    #     self.x_lst = self.x_lst[highest_idx:longest_idx]
    #     self.y_lst = self.y_lst[highest_idx:longest_idx]
    #     self.u = self.u[:, highest_idx:longest_idx]
    #     self.one_minus_u = self.one_minus_u[:, highest_idx:longest_idx]
    #     self.num_of_samples = len(self.x_lst)

    def get_dKdx(self):
        if self.dKdx_lst is None:
            if self.curvaute_lst is None:
                self.get_curvatured_lst()
            if self.dx_lst is None:
                self.get_arc_length_lst()

            dK = np.diff(self.curvaute_lst)
            self.dKdx_lst = dK / self.dx_lst
        return self.dKdx_lst

    def get_Torque_lst(self, rho_g):
        if self.ds_lst is None:
            self.get_arc_length_lst()
        #  self.num_of_samples
        num_of_seg = self.num_of_samples - 1

        sum_mass_rhog_from_x_to_end = 0
        sum_mass_rhog_prod_x_from_x_to_end = 0
        torque_lst = [0]
        for i in reversed(range(0, num_of_seg)):
            sum_mass_rhog_from_x_to_end += self.ds_lst[i] * rho_g
            sum_mass_rhog_prod_x_from_x_to_end += self.ds_lst[i] * rho_g * (
                self.x_lst[i] + self.x_lst[i + 1]) / 2
            COM = sum_mass_rhog_prod_x_from_x_to_end / sum_mass_rhog_from_x_to_end
            force_arm = COM - self.x_lst[i]
            force = sum_mass_rhog_from_x_to_end
            torque = force_arm * force
            torque_lst.append(torque)
        torque_lst.reverse()
        #     print(i, torque)
        # print(f"x num {len(self.x_lst)}")
        # exit()
        return torque_lst

    def __calc_discrete_point(self):
        '''
        u = linspace(0, 1, 200)
        1_minus_u = 1 - u
        r(t) =  a * pow(1_minus_u, 3) 
                + 3 * b * u * pow(1_minus_u, 2)
                + 3 * c * u^2 * pow(1_minus_u, 1)
                + d * u^3
        '''

        sum = None
        for i in range(self.order + 1):
            val = Com(self.order, i) * np.dot(
                self.bezier_points[i].T,
                np.power(self.u, i) *
                np.power(self.one_minus_u, self.order - i))
            if sum is None:
                sum = val
            else:
                sum += val

        return list(np.squeeze(sum[0])), list(np.squeeze(sum[1]))

    def get_discretized_point_lst(self):
        return self.x_lst, self.y_lst

    def __calc_curvature_lst(self):
        '''
            K(t = |r'(t) \times r''(t)| / (|r'(t)|^3)

            r'(t) 
                = drdt
                = 3 * {
                    [3 * (B - C) + D - A] t^2 
                    + 2 ( A- 2 B + C) t
                    - A + B
                }
                        
            r''(t)
                = drdt 
                = 6 *{
                    [3 (B - C) + D - A]t 
                    + A - 2 B + C
                    }
                
        '''
        u2 = np.power(self.u, 2)
        A, B, C, D = self.bezier_points[0].T, self.bezier_points[
            1].T, self.bezier_points[2].T, self.bezier_points[3].T

        # 1. get r'(t)
        r_prime = 3 * ((3 * (B - C) + D - A) * u2 + 2 *
                       (A - 2 * B + C) * self.u - A + B)
        # 2. get r''(t)
        r_prime2 = 6 * ((3 * (B - C) + D - A) * self.u + A - 2 * B + C)
        r_prime2_pow3 = np.array(np.abs(np.linalg.norm(r_prime, axis=0))**3)

        # 3. do cross product, and divice
        r_prime = list(r_prime.T)
        r_prime2 = list(r_prime2.T)
        rprime_cross_rprime2 = np.abs(np.cross(list(r_prime), list(r_prime2)))

        return rprime_cross_rprime2 / r_prime2_pow3

    def get_curvatured_lst(self):

        return self.curvaute_lst

    def debug_draw_bezier(self):
        max_x = np.max(self.x_lst)
        max_y = np.max(self.y_lst)
        max_v = max(max_x, max_y)
        from matplotlib import pyplot as plt
        plt.plot(self.x_lst, self.y_lst)
        plt.xlim(-max_v, max_v)
        plt.ylim(-max_v, max_v)
        plt.gca().set_aspect('equal', adjustable='box')
        plt.draw()
        plt.show()

    def get_arc_length_lst(self):
        if self.ds_lst is None:
            self.dx_lst = np.diff(self.x_lst)
            self.dy_lst = np.diff(self.y_lst)
            self.ds_lst = np.power(self.dx_lst**2 + self.dy_lst**2, 0.5)
            self.s_lst = []
            self.total_arc_length = 0
            for i in self.ds_lst:
                self.s_lst.append(self.total_arc_length)
                self.total_arc_length += i

        return self.ds_lst

    def get_total_arc_length(self):
        if self.total_arc_length is None:
            self.get_arc_length_lst()
        return self.total_arc_length

    def get_arc_integral_over_x_to_free_end(self):
        '''
            calculate the integral F(t) = \int_{t}^{t_{end}} s(x) dx
            t_end: the x coords of free end
            t: the x coords of any point
            s(x): the arc length from origin to this point (with x coords = 'x')
        '''

    def get_dTorquedx(self, rho_g):
        if self.s_lst is None:
            self.get_arc_length_lst()

        return list( rho_g * (np.array( self.s_lst) - self.total_arc_length))

    def get_dTorquedx_new(self, rho_g):
        M = self.get_Torque_lst(rho_g)
        dMdx = np.diff(M) / self.dx_lst
        return dMdx

    def get_theta_lst(self):
        if self.theta_lst is None:
            self.theta_lst = np.arctan(self.dy_lst / self.dx_lst)
        return self.theta_lst


if __name__ == "__main__":
    from load_bending_bezier_data import load_bezier_datamat
    from calculate_bending_stiffness_from_bezier import normalize_from_pixel_to_meter, get_linear_density_for_specimen, move_the_bezier_origin_and_mirror
    bezier_data_path = "D:\Projects\弯曲测量数据\\1\Back-0.mat"
    A, B, C, D, unit_cm, img_filename, projective2d = load_bezier_datamat(
        bezier_data_path)

    rho_g = get_linear_density_for_specimen("D:\Projects\弯曲测量数据\\1") * 9.8
    A, B, C, D = normalize_from_pixel_to_meter(unit_cm, A, B, C, D)
    A, B, C, D = move_the_bezier_origin_and_mirror(A, B, C, D)

    bezier_curve = BezierCurve(A, B, C, D)
    # 1. verify the curvature: correct
    x_lst, y_lst = bezier_curve.get_discretized_point_lst()
    K_lst = bezier_curve.get_curvatured_lst()
    ds_lst = bezier_curve.get_arc_length_lst()
    # bezier_curve.debug_draw_bezier()
    # exit()
    for i in range(1, len(K_lst) - 1):
        K = K_lst[i]
        p0 = np.array([x_lst[i - 1], y_lst[i - 1]])
        p1 = np.array([x_lst[i], y_lst[i]])
        p2 = np.array([x_lst[i + 1], y_lst[i + 1]])
        v0 = p1 - p0
        v1 = p2 - p1
        v0_normed = v0 / np.linalg.norm(v0)
        v1_normed = v1 / np.linalg.norm(v1)

        theta_changed = np.arccos(np.dot(v0_normed, v1_normed))
        arc_length = ds_lst[i - 1]
        discrete_K = theta_changed / arc_length
        # print(f"K {K} dis K {discrete_K}")

    # 2. hard and give up: verify the arc length (from the analytic integration)
    # 3. try to verify
    '''
        3.1: dM/ds = rho * g (s - l) * dx / ds : verify done
    '''
    if False:
        dMdx = np.diff(K_lst) / np.diff(x_lst)
        s_minus_l = bezier_curve.s_lst - bezier_curve.total_arc_length
        dxds = np.diff(x_lst)[:-1] / np.diff(bezier_curve.s_lst)

        M_lst = bezier_curve.get_Torque_lst(rho_g)
        dMds_def = np.diff(M_lst) / ds_lst
        dMds_formal = rho_g * (s_minus_l[:-1] * dxds)
        print(f"dMds def {dMds_def}")
        print(f"dMds form {dMds_formal}")
    '''
        3.2 dK/ds = [ atan(\sqrt{(ds/dx)**2 - 1}) + 2 * \theta_t - \theta_{t-1}] / ds^2
    '''
    if True:
        # 1. calculate dKds
        theta_lst = bezier_curve.get_theta_lst()
        theta_pred_lst = np.arctan(
            np.sqrt((ds_lst / bezier_curve.dx_lst)**2 - 1))
        # print(f"theta lst {theta_lst}")
        # print(f"theta_pred_lst {theta_pred_lst}")
        dKds_def = np.diff(K_lst) / ds_lst
        dKds_formal = []
        for i in range(1, len(theta_pred_lst) - 1):
            theta_next = theta_pred_lst[i + 1]
            theta_cur = theta_pred_lst[i]
            theta_prev = theta_pred_lst[i - 1]
            dKds = ((theta_next - theta_cur) / (ds_lst[i]) -
                    (theta_cur - theta_prev) /
                    (ds_lst[i - 1])) / ((ds_lst[i] + ds_lst[i - 1]) / 2)
            # print(dKds)
            # exit()
            dKds_formal.append(dKds)
            # dKds = (theta_next + 2 * theta_cur - theta_prev) / (ds_lst[i]**2)
        print(f"dKds def {list(dKds_def)}")
        print(f"dKds formal {dKds_formal}")
        # for i in range(1, len( bezier_curve.x_lst) - 1):
        #     theta_next_pred_lst[i + 1]
        # dK = np.diff(bezier_curve.curvaute_lst)
        # ds = np.diff(bezier_curve.s_lst)
        # dKds_def = dK / ds
