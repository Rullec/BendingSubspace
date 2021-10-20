#include "BezierShootSolver.h"
#include "utils/LogUtil.h"

/**
 * \brief       for more details, please check the note "20211019 1.9 shooting method robust method"
*/
double NumeriInte(double theta0, double beta, int num_samples, double guess_xc, tVectorXd &x,
                  tVectorXd &y)
{
    tVectorXd s = tVectorXd::LinSpaced(num_samples, 0, 1);

    double ds = s(2) - s(1);
    x = tVectorXd::Zero(num_samples);
    y = tVectorXd::Zero(num_samples);

    /*
    
        K =  M / G = Xc * rho_g_L / G > 0
        now, Xc should be Xc / L^2 = guess_xc_xcoords * 100
    */
    double K = guess_xc * beta;

    /**
     * 
    */
    double dydx = std::tan(theta0);
    // double ddydxds = -K * (1 + dydx * dydx);

    for (int i = 0; i < s.size() - 1; i++)
    {

        double dx = ds / std::sqrt(1 + dydx * dydx);
        double dy = (dydx < 0 ? -1 : 1) * ds * 1.0 / sqrt(1 + 1.0 / (dydx * dydx));

        x(i + 1) = x(i) + dx; // correct
        y(i + 1) = y(i) + dy; // correct

        double new_K = K + beta * (s[i + 1] - 1) * dx;
        K = std::max(0.0, new_K);

        dydx = dydx - K * (1 + dydx * dydx) * ds;
    }
    double delta = (x * ds).sum() - guess_xc;
    return delta;
}
double cBezierShootSolver::ShootLinearSolveRobust(double beta, double theta0, int num_of_samples, tVectorXd &x_solved,
                                                  tVectorXd &y_solved)
{
    SIM_ASSERT(beta > 0);
    double guess_xc = 0.25;
    double guess_xc_prev = guess_xc * 0.99;
    double delta = NumeriInte(theta0, beta, num_of_samples, guess_xc, x_solved, y_solved);

    double delta_prev = NumeriInte(theta0, beta, num_of_samples, guess_xc * 0.99, x_solved, y_solved);

    // double step = 0.25;
    double stepsize = 1;
    int cur_iter = 0;
    while (std::fabs(delta) > 1e-5)
    {
        // step = (delta > 0 ? 1 : -1) * std::fabs(step) / 2;
        double step = -stepsize * delta * (guess_xc - guess_xc_prev) / (delta - delta_prev);
        guess_xc_prev = guess_xc;
        guess_xc += step;

        delta_prev = delta;
        delta = NumeriInte(theta0, beta, num_of_samples, guess_xc, x_solved, y_solved);
        std::cout << "[linear] cur iter " << cur_iter << " err = " << delta << std::endl;
        cur_iter += 1;
    }
    return std::fabs(delta);
}
