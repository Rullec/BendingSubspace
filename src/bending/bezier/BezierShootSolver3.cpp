#include "BezierShootSolver.h"

static double NumeriInte(double rho_g, double a, double b, double total_length, double theta0, double com_pos, int num_of_samples, tVectorXd &x,
                         tVectorXd &y, bool select_k1 = true)
{
    tVectorXd s = tVectorXd::LinSpaced(num_of_samples, 0, total_length);
    double ds = s(1) - s(0);
    x = tVectorXd::Zero(num_of_samples);
    y = tVectorXd::Zero(num_of_samples);

    // M = F * S = (rho * g * L) * com_pos
    double M = rho_g * total_length * com_pos;

    // M = aK^2 + bK
    double delta = b * b + 4 * a * M;
    if (delta < 0)
    {
        printf("a = %.1e, b = %.1e, M = %.3f, delta = %.1e, no solution for curvature", a, b, M, delta);
        exit(1);
    }

    double K = 0;
    if (a > 0)
    {
        K = (-b + std::sqrt(b * b + 4 * a * M)) / (2 * a);
    }
    else
    {
        // it is all valid ok when a < 0
        double K1 = (-b + std::sqrt(b * b + 4 * a * M)) / (2 * a);
        double K2 = (-b - std::sqrt(b * b + 4 * a * M)) / (2 * a);
        std::cout << "[debug] two option, K1 = " << K1 << " k2 = " << K2 << std::endl;
        // std::cout << "a = " << a << " b = " << b << " M = " << M << std::endl;
        // std::cout << "b^2 = " << b * b << " 4aM = " << 4 * a * M << " delta = " << (b * b + 4 * a * M) << std::endl;
        // std::cout << "-b = " << -b << " sqrt{delta} = " << std::sqrt(b * b + 4 * a * M) << " -b + sqrt{delta} = " << -b + std::sqrt(b * b + 4 * a * M) << std::endl;
        // exit(1);
        if (select_k1)
        {
            K = K1;
        }
        else
        {
            K = K2;
        }
        // if (K2 > 0)
        // {
        //     K = K1;
        // }
        // else if (K1 > 0)
        // {
        //     K = K2;
        // }
        // else
        // {
        //     std::cout << "k1 = " << K1 << " k2 = " << K2 << std::endl;
        //     exit(1);
        // }
        // std::cout << "select K = " << K << std::endl;
    }

    double dydx = std::tan(theta0);

    for (int i = 0; i < s.size() - 1; i++)
    {
        // 1. calculate dx and dy
        double dx = ds / std::sqrt(1 + dydx * dydx);
        double dy = (dydx < 0 ? -1 : 1) * ds * sqrt(1 - 1.0 / (1 + dydx * dydx));

        x(i + 1) = x(i) + dx; // correct
        y(i + 1) = y(i) + dy; // correct

        // 2. update K and dydx
        double dKdx = rho_g * (s[i] - total_length) / (2 * a * K + b);
        double new_K = K + dKdx * dx;
        K = std::max(0.0, new_K);

        double ddydxds = -K * (1 + dydx * dydx);
        dydx = dydx + ddydxds * ds;
    }

    return std::fabs(x.sum() / num_of_samples - com_pos);
}
/**
 * \brief       
*/
#include "utils/LogUtil.h"
double cBezierShootSolver::ShootNonLinearSolveRobust(double rho_g, double a, double b, double total_length, double theta0, int num_of_samples, tVectorXd &x_solved,
                                                     tVectorXd &y_solved, bool select_k1)
{
    double guess_xc = 0;
    double min_xc = 0;
    double max_xc = 1;
    if (a > 0)
    {
        guess_xc = total_length / 2;
    }
    else
    {
        max_xc = -b * b / (4 * a * rho_g * total_length);
        max_xc = std::min(max_xc, total_length / 2);
        std::cout << "[debug] max com pos = " << max_xc << std::endl;
        guess_xc = max_xc / 2;
        // Xc
    }
    double guess_xc_prev = guess_xc * 0.99;
    double func_prev = NumeriInte(rho_g, a, b, total_length, theta0, guess_xc_prev, num_of_samples, x_solved, y_solved, select_k1);
    double func_cur = NumeriInte(rho_g, a, b, total_length, theta0, guess_xc, num_of_samples, x_solved, y_solved, select_k1);

    double stepsize = 0.1;
    int cur_iter = 0;
    int max_iter = 500;
    std::cout << "[nonlinear] start com = " << guess_xc << " err = " << func_cur << std::endl;
    while (std::fabs(func_cur) > 1e-4 && cur_iter < max_iter)
    {
        // if (cur_iter % (max_iter / 2) == 0 && cur_iter != 0)
        // {
        //     stepsize *= 0.1;
        // }
        double step = -stepsize * func_cur * (guess_xc - guess_xc_prev) / (func_cur - func_prev);
        guess_xc_prev = guess_xc;
        guess_xc += step;

        if (guess_xc < 0 || guess_xc > max_xc)
        {

            std::cout << "[debug] guess_xc = " << guess_xc << " out of range [0," << max_xc << "]\n";
            guess_xc = cMathUtil::RandDouble(0, max_xc);
            std::cout << "[debug] reset to " << guess_xc << std::endl;
        }
        func_prev = func_cur;
        func_cur = NumeriInte(rho_g, a, b, total_length, theta0, guess_xc, num_of_samples, x_solved, y_solved, select_k1);
        if (x_solved.hasNaN() == true)
        {
            std::cout << "x solved has Nan, exit\n";
            exit(1);
        }
        if (y_solved.hasNaN() == true)
        {
            std::cout << "y solved has Nan, exit\n";
            exit(1);
        }
        std::cout << "[nonlinear] cur iter " << cur_iter << " com = " << guess_xc << " err = " << func_cur << std::endl;
        cur_iter += 1;
    }
    // std::cout << "y = " << y_solved.transpose() << std::endl;
    // std::cout << "x = " << x_solved.transpose() << std::endl;
    return std::fabs(func_cur);
}