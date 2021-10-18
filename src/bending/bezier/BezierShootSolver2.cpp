#include "BezierShootSolver.h"

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
    double dKdx = beta * (s(1) - 1);
    double dydx = std::tan(theta0);
    double ddydxds = -K * (1 + dydx * dydx);

    for (int i = 0; i < s.size() - 1; i++)
    {

        double dx = ds / std::sqrt(1 + dydx * dydx); // 正确
        // double dy = (dydx < 0 ? -1 : 1) * ds * sqrt(1 - 1 / (1 + dydx * dydx)); // wenchao
        double dy = (dydx < 0 ? -1 : 1) * ds * 1.0 / sqrt(1 + 1.0 / (dydx * dydx)); // mine

        x(i + 1) = x(i) + dx; // correct
        y(i + 1) = y(i) + dy; // correct

        K = std::max(0.0, K + dKdx * dx);
        dydx = dydx + ddydxds * ds;
        dKdx = beta * (s(i + 1) - 1);
        ddydxds = -K * (1 + dydx * dydx);
    }
    double delta = (x * ds).sum() - guess_xc;
    return delta;
}
double cBezierShootSolver::ShootLinearSolveWenchao(double beta, double theta0, int num_of_samples, tVectorXd &x_solved,
                                                   tVectorXd &y_solved)
{
    double guess_xc = 0.25;

    double delta = NumeriInte(theta0, beta, num_of_samples, guess_xc, x_solved, y_solved);

    double step = 0.25;
    double epsilon = delta;

    while (std::fabs(delta) > 1e-5)
    {
        step = (delta > 0 ? 1 : -1) * std::fabs(step) / 2;
        guess_xc += step;

        double new_delta = NumeriInte(theta0, beta, num_of_samples, guess_xc, x_solved, y_solved);
        epsilon = new_delta - delta;
        delta = new_delta;
    }
    return std::fabs(epsilon);
}