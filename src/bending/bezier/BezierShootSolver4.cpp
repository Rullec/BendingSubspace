#include "BezierShootSolver.h"

static double NumerInte(double com_pos, double rho_g, double a, double b, double total_length, double theta0, int num_of_samples, tVectorXd &x,
                        tVectorXd &y, bool select_k1)
{
    tVectorXd s = tVectorXd::LinSpaced(num_of_samples, 0, 1);
    double ds = s(1) - s(0);
    x = tVectorXd::Zero(num_of_samples);
    y = tVectorXd::Zero(num_of_samples);

    // 1. get init K
    double K = 0;
    double discriminant = b * b + 4 * a * rho_g * total_length * total_length * com_pos;
    if (discriminant < 0)
    {
        std::cout << "[error] discriminant = " << discriminant << " < 0 \n";
        exit(1);
    }
    if (a > 0)
    {
        K = total_length * (-b + std::sqrt(discriminant)) / (2 * a);
    }
    else
    {
        // a < 0
        if (select_k1)
        {
            K = total_length * (-b + std::sqrt(discriminant)) / (2 * a);
        }
        else
        {
            K = total_length * (-b - std::sqrt(discriminant)) / (2 * a);
        }
    }

    // 2. begin to iter
    double dydx = std::tan(theta0);
    double m = 2 * a / (rho_g * std::pow(total_length, 4));
    double n = b / (rho_g * std::pow(total_length, 3));
    for (int i = 0; i < s.size() - 1; i++)
    {
        double dx = ds / std::sqrt(1 + dydx * dydx);
        double dy = (dydx < 0 ? -1 : 1) * ds * sqrt(1 - 1.0 / (1 + dydx * dydx));

        x(i + 1) = x(i) + dx; // correct
        y(i + 1) = y(i) + dy; // correct

        double dKdx = 1.0 / (m * K + n) * (s[i] - 1);
        double new_k = K + dKdx * dx;
        K = std::max(0.0, new_k);

        double ddydxds = -K * (1 + dydx * dydx);
        dydx = dydx + ddydxds * ds;
    }

    return std::fabs((x * ds).sum() - com_pos);
}
double cBezierShootSolver::ShootNonLinearSolveRobustNormalized(double rho_g, double a, double b, double total_length, double theta0, int num_of_samples, tVectorXd &x_solved,
                                                               tVectorXd &y_solved, bool select_k1)
{
    // std::cout << "a = " << a << std::endl;
    // std::cout << "b = " << b << std::endl;
    // std::cout << "total_length = " << total_length << std::endl;
    // std::cout << "theta0 = " << theta0 << std::endl;
    // std::cout << "rho_g = " << rho_g << std::endl;
    // 1. give the range
    double max_c = 1.0 / 2, min_c = 0;
    if (a < 0)
    {
        max_c = std::min(
            max_c,
            -b * b / (4 * a * rho_g * total_length * total_length));
    }

    // 2. warm up the newton method
    double com_cur = (max_c + min_c) / 2;
    double com_prev = com_cur * 0.99;

    double func_cur = NumerInte(com_cur, rho_g, a, b, total_length, theta0, num_of_samples, x_solved, y_solved, select_k1);
    double func_prev = NumerInte(com_prev, rho_g, a, b, total_length, theta0, num_of_samples, x_solved, y_solved, select_k1);

    int cur_iter = 0;
    int max_iter = 500;
    double eps = 1e-5;
    double stepsize = 0.1;
    std::cout << "[nonlinear_normed] start com = " << com_cur << " err = " << func_cur << std::endl;

    // 3. begin to loop
    while (std::fabs(func_cur) > eps && cur_iter < max_iter)
    {

        double step = -stepsize * func_cur * (com_cur - com_prev) / (func_cur - func_prev);

        com_prev = com_cur;
        com_cur += step;

        // if out of range, reset and re-evaluate
        if (com_cur < min_c || com_cur > max_c)
        {
            // reset
            std::cout << "[debug_normed] com = " << com_cur << " out of range [0," << max_c << "]\n";
            com_cur = cMathUtil::RandDouble(0, max_c);
            std::cout << "[debug_normed] reset to " << com_cur << std::endl;
            com_prev = com_cur * 0.99;
            // re-evaluate

            func_prev = NumerInte(com_prev, rho_g, a, b, total_length, theta0, num_of_samples, x_solved, y_solved, select_k1);
        }
        else
        {
            // inside of the range
            func_prev = func_cur;
        }
        func_cur = NumerInte(com_cur, rho_g, a, b, total_length, theta0, num_of_samples, x_solved, y_solved, select_k1);
        std::cout << "[nonlinear_normed] cur iter " << cur_iter << " com = " << com_cur << " func = " << func_cur << std::endl;
        cur_iter += 1;
    }
    x_solved *= total_length;
    y_solved *= total_length;
    return std::fabs(func_cur);
}