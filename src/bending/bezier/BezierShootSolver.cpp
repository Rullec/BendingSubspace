#include "BezierShootSolver.h"
#include "utils/LogUtil.h"
#define M_PI 3.1415926535
/**
 * \brief               Begin to solve guess IVP normalized
*/
template <typename T>
double sum_vec(const std::vector<T> &vec)
{
    double sum = 0;
    for (auto &x : vec)
        sum += x;
    return sum;
}

tVectorXd StdVecToEigenVec(const std::vector<double> &vec)
{
    tVectorXd eigen_vec(vec.size());
    for (int i = 0; i < vec.size(); i++)
    {
        eigen_vec[i] = vec[i];
    }
    return eigen_vec;
}
#include "utils/LogUtil.h"
double calculate_angle_error(const std::vector<double> &theta_lst)
{
    double err_angle = 0;
    double min_angle = -M_PI / 2 * 1.1;
    for (int idx = 0; idx < theta_lst.size(); idx++)
    {
        auto cur_theta = theta_lst[idx];

        double incre = 0;
        if (cur_theta > 0)
        {
            SIM_ASSERT(cur_theta > 0);
            err_angle += cur_theta;
        }
        else if (cur_theta < min_angle)
        {
            SIM_ASSERT(min_angle - cur_theta > 0);
            err_angle += min_angle - cur_theta;
        }

        if (idx > 0)
        {
            double diff = theta_lst[idx] - theta_lst[idx - 1];
            if (diff > 0)
            {
                // std::cout << "idx " << idx << " diff = " << diff << std::endl;
                err_angle += diff;
            }
        }
    }
    // std::cout << "total err = " << err_angle << std::endl;
    return err_angle;
}

double implicit_eval(double x, double h2_beta_s_minus_1, double theta_t, double minus_h_beta_M)
{
    return x - h2_beta_s_minus_1 * std::cos(x) - theta_t + minus_h_beta_M;
}

double implicit_dfdx_eval(double x, double h2_beta_s_minus_1)
{
    return 1 + h2_beta_s_minus_1 * std::sin(x);
}

double implicit_newton(double h2_beta_s_minus_1, double theta_t, double minus_h_beta_M)
{
    double x_min = -M_PI / 2;
    double x_max = 0;
    double x_cur = (x_max + x_min) / 2;
    double func_cur = implicit_eval(x_cur, h2_beta_s_minus_1, theta_t, minus_h_beta_M);
    double eps = 1e-9;
    double dfdx = implicit_dfdx_eval(x_cur, h2_beta_s_minus_1);
    if (std::fabs(func_cur) < eps)
    {
        return x_cur;
    }
    // std::cout << "[iter] -1 "
    //           << " x_cur = " << x_cur << " func = " << func_cur << " dfdx = " << dfdx << std::endl;
    // std::cout << "h2_beta_s_minus_1 = " << h2_beta_s_minus_1 << std::endl;
    int max_iter = 100;
    for (int _i = 0; _i < max_iter; _i++)
    {
        x_cur = x_cur - 1.0 / dfdx * func_cur;
        func_cur = implicit_eval(x_cur, h2_beta_s_minus_1, theta_t, minus_h_beta_M);
        dfdx = implicit_dfdx_eval(x_cur, h2_beta_s_minus_1);
        // std::cout << "[iter] " << _i << " x_cur = " << x_cur << " func = " << func_cur << " dfdx = " << dfdx << std::endl;
        if (std::fabs(func_cur) < eps)
        {
            break;
        }
    }
    return x_cur;
}
double solve_guess_IVP_normalized_linear(double beta, double theta0, double x0, double y0, double t, tVectorXd &eigen_x_lst, tVectorXd &eigen_y_lst)
{
    std::vector<double> M_lst(0), theta_lst(0), x_lst(0), y_lst(0);
    int steps = 500;
    double h = 1.0 / (steps - 1);

    double x_cur = x0;
    double y_cur = y0;
    double s_cur = 0;
    double theta_cur = theta0;
    double M_cur = t;

    for (int i = 0; i < steps; i++)
    {
        M_lst.push_back(M_cur);
        theta_lst.push_back(theta_cur);
        x_lst.push_back(x_cur);
        y_lst.push_back(y_cur);

        // explicit euler
        // {
        //     double theta_new = theta_cur + beta * M_cur * h;
        //     double M_new = M_cur + h * (s_cur - 1) * std::cos(theta_cur);
        //     theta_cur = theta_new;
        //     M_cur = M_new;
        // }

        // implicit euler

        {
            double h2_beta_s_minus_1 = h * h * beta * (s_cur + h - 1);
            double minus_h_beta_M = -h * beta * M_cur;

            double theta_new = implicit_newton(h2_beta_s_minus_1, theta_cur, minus_h_beta_M);
            double theta_new_exp = h * beta * M_cur + theta_cur;
            // std::cout << "theta new explicit = " << theta_new_exp << std::endl;
            // std::cout << "theta new implicit = " << theta_new << std::endl;
            // if (theta_new >= 0 || theta_new < -M_PI / 2)
            // {
            //     std::cout << "theta new = " << theta_new << " illegal!\n";
            //     exit(1);
            // }
            M_cur = (theta_new - theta_cur) / (h * beta);
            theta_cur = theta_new;
        }

        s_cur += h;
        x_cur += h * std::cos(theta_cur);
        y_cur += h * std::sin(theta_cur);
    }

    double err_t = std::fabs(h * sum_vec(x_lst) - t);
    // double err_angle = calculate_angle_error(theta_lst) * h;
    double err_angle = calculate_angle_error(theta_lst);
    // std::cout << "err_t = " << err_t << std::endl;
    // std::cout << "err_angle = " << err_angle << std::endl;
    double err = err_t + err_angle;

    eigen_x_lst = StdVecToEigenVec(x_lst);
    eigen_y_lst = StdVecToEigenVec(y_lst);

    return err;
}

/**
 * \brief           solve bending strip by shooting method
 * the input cruve has been arclength normalized (L = 0 ~ 1)
 * beta = - rho * g  * L^3 / G
 *  L is the total arclength
 *  G is the bending stiffness
 *  t0 is the torque guess in start point
*/
double cBezierShootSolver::ShootLinearSolve(
    double beta, double theta0, double t0, double stepsize, tVectorXd &x_lst_solved, tVectorXd &y_lst_solved, bool silent /* = false*/)
{
    double t1 = 0.99 * t0;
    double eps = 1e-3;
    double x0 = 0, y0 = 0;

    double t_prev = t0;
    double t_cur = t1;

    // give 2 shoot at first (warm start for newton method)
    tVectorXd x_lst_prev, y_lst_prev;
    tVectorXd x_lst_cur, y_lst_cur;
    double err_prev = solve_guess_IVP_normalized_linear(
        beta, theta0, x0, y0, t_prev, x_lst_prev, y_lst_prev);
    double err_cur = solve_guess_IVP_normalized_linear(
        beta, theta0, x0, y0, t_cur, x_lst_cur, y_lst_cur);
    // std::cout << "err prev = " << err_prev << std::endl;
    // std::cout << "err cur = " << err_cur << std::endl;

    if (std::fabs(err_prev) < eps)
    {
        x_lst_solved = x_lst_prev;
        y_lst_solved = y_lst_prev;
        return err_prev;
    }
    if (std::fabs(err_cur) < eps)
    {
        x_lst_solved = x_lst_cur;
        y_lst_solved = y_lst_cur;
        return err_cur;
    }

    // if these two shots are not good, begin to iter
    int max_shooting = 2000;
    double min_err = 1e10;
    tVectorXd min_err_x, min_err_y;
    tVectorXd x_tmp, y_tmp;

    int k = 0;
    for (k = 0; k < max_shooting + 1; k++)
    {
        // std::cout << "[cpp] old t = " << t_cur << ", stepsize = " << stepsize << ", err_cur = " << err_cur << " dt = " << (t_cur - t_prev) << ", de = " << (err_cur - err_prev) << std::endl;
        double dt = -stepsize * err_cur * (t_cur - t_prev) / (err_cur - err_prev);
        // std::cout << "[cpp] Delta t = " << dt << std::endl;
        // t_prev = t_cur;
        t_cur = t_cur + dt;
        // std::cout << "[cpp] new t = " << t_cur << std::endl;
        // err_prev = err_cur;
        err_cur = solve_guess_IVP_normalized_linear(beta, theta0, x0, y0, t_cur, x_tmp, y_tmp);

        // std::cout << "[cpp] iter " << k << " new t = " << t_cur << " err_cur " << err_cur << std::endl;
        if (silent == false)
        {
            if ((k % 500 == 0) || err_cur < eps)
            {
                std::cout << "[debug] iter " << k << " t " << t_cur << ", t0 = " << t0 << " err = " << err_cur << std::endl;
                ;
            }
        }
        if (err_cur < min_err)
        {
            min_err = err_cur;
            min_err_x = x_tmp;
            min_err_y = y_tmp;
        }
        if (err_cur < eps)
            break;
    }
    // exit(1);
    if (k == max_shooting)
    {
        printf("[error] solved failed, get min err %.3f > eps %.3f\n", min_err, eps);

        x_lst_solved = min_err_x;
        y_lst_solved = min_err_y;
    }
    else
    {
        x_lst_solved = min_err_x;
        y_lst_solved = min_err_y;
    }

    return min_err;
}

double solve_guess_IVP_normalized_nonlinear(double m, double n, double theta0, double x0, double y0, double t, tVectorXd &eigen_x_lst, tVectorXd &eigen_y_lst)
{
    std::vector<double> M_lst(0), theta_lst(0), x_lst(0), y_lst(0);
    int steps = 500;
    double h = 1.0 / (steps - 1);

    double x_cur = x0;
    double y_cur = y0;
    double s_cur = 0;
    double theta_cur = theta0;
    double M_cur = t;

    for (int i = 0; i < steps; i++)
    {
        M_lst.push_back(M_cur);
        theta_lst.push_back(theta_cur);
        x_lst.push_back(x_cur);
        y_lst.push_back(y_cur);

        // explicit euler
        {

            double M_new = h * (s_cur - 1) * std::cos(theta_cur) + M_cur;
            double theta_new = theta_cur - h * ((n + std::sqrt(n * n + 4 * m * M_new)) / (2 * m));
            M_cur = M_new;
            theta_cur = theta_new;
        }

        // implicit euler

        s_cur += h;
        x_cur += h * std::cos(theta_cur);
        y_cur += h * std::sin(theta_cur);
    }

    double err_t = std::fabs(h * sum_vec(x_lst) - t);
    // double err_angle = calculate_angle_error(theta_lst) * h;
    double err_angle = calculate_angle_error(theta_lst);
    // std::cout << "err_t = " << err_t << std::endl;
    // std::cout << "err_angle = " << err_angle << std::endl;
    double err = err_t + err_angle;

    eigen_x_lst = StdVecToEigenVec(x_lst);
    eigen_y_lst = StdVecToEigenVec(y_lst);

    return err;
}

double cBezierShootSolver::ShootNonLinearSolve(double m, double n, double theta0, double t0, double stepsize,
                                               tVectorXd &x_lst_solved,
                                               tVectorXd &y_lst_solved, bool silent)
{
    double t1 = 0.99 * t0;
    double eps = 1e-3;
    double x0 = 0, y0 = 0;

    double t_prev = t0;
    double t_cur = t1;

    // give 2 shoot at first (warm start for newton method)
    tVectorXd x_lst_prev, y_lst_prev;
    tVectorXd x_lst_cur, y_lst_cur;
    double err_prev = solve_guess_IVP_normalized_nonlinear(
        m, n, theta0, x0, y0, t_prev, x_lst_prev, y_lst_prev);
    double err_cur = solve_guess_IVP_normalized_nonlinear(
        m, n, theta0, x0, y0, t_cur, x_lst_cur, y_lst_cur);
    // std::cout << "err prev = " << err_prev << std::endl;
    // std::cout << "err cur = " << err_cur << std::endl;

    if (std::fabs(err_prev) < eps)
    {
        x_lst_solved = x_lst_prev;
        y_lst_solved = y_lst_prev;
        return err_prev;
    }
    if (std::fabs(err_cur) < eps)
    {
        x_lst_solved = x_lst_cur;
        y_lst_solved = y_lst_cur;
        return err_cur;
    }

    // if these two shots are not good, begin to iter
    int max_shooting = 2000;
    double min_err = 1e10;
    tVectorXd min_err_x, min_err_y;
    tVectorXd x_tmp, y_tmp;

    int k = 0;
    for (k = 0; k < max_shooting + 1; k++)
    {
        // std::cout << "[cpp] old t = " << t_cur << ", stepsize = " << stepsize << ", err_cur = " << err_cur << " dt = " << (t_cur - t_prev) << ", de = " << (err_cur - err_prev) << std::endl;
        double dt = -stepsize * err_cur * (t_cur - t_prev) / (err_cur - err_prev);
        // std::cout << "[cpp] Delta t = " << dt << std::endl;
        // t_prev = t_cur;
        t_cur = t_cur + dt;
        if (m < 0)
        {
            t_cur = std::min(t_cur, -n * n / (4 * m));
        }

        // std::cout << "[cpp] new t = " << t_cur << std::endl;
        // err_prev = err_cur;
        err_cur = solve_guess_IVP_normalized_nonlinear(m, n, theta0, x0, y0, t_cur, x_tmp, y_tmp);

        // std::cout << "[cpp] iter " << k << " new t = " << t_cur << " err_cur " << err_cur << std::endl;
        if (silent == false)
        {
            if ((k % 500 == 0) || err_cur < eps)
            {
                std::cout << "[debug] iter " << k << " t " << t_cur << ", t0 = " << t0 << " err = " << err_cur << std::endl;
                ;
            }
        }
        if (err_cur < min_err)
        {
            min_err = err_cur;
            min_err_x = x_tmp;
            min_err_y = y_tmp;
        }
        if (err_cur < eps)
            break;
    }
    // exit(1);
    if (k == max_shooting)
    {
        printf("[error] solved failed, get min err %.3f > eps %.3f\n", min_err, eps);

        x_lst_solved = min_err_x;
        y_lst_solved = min_err_y;
    }
    else
    {
        x_lst_solved = min_err_x;
        y_lst_solved = min_err_y;
    }

    return min_err;
}