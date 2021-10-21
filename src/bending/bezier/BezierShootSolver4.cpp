#include "BezierShootSolver.h"
// [x, y, s, delta] =
double NumericalIntegrate(double theta0_deg, double len, double alpha_invRho_invG, double beta_invRho_invG, int numSamples, double guessXc,
                          tVectorXd &x_solved, tVectorXd &y_solved)
{

    tVectorXd s = tVectorXd::LinSpaced(numSamples, 0, len);

    double ds = s(2) - s(1);
    x_solved.noalias() = tVectorXd::Zero(numSamples);
    y_solved.noalias() = tVectorXd::Zero(numSamples);

    double K = 0;
    double M_invrho_invG = len * len * guessXc - 0;
    // if (beta_invRho_invG == 0)
    // {
    //     K = M_invrho_invG / alpha_invRho_invG;
    // }
    // else
    // {
    double K_part1 = std::sqrt(
        std::max(
            0.0,
            alpha_invRho_invG * alpha_invRho_invG + 4 * beta_invRho_invG * M_invrho_invG));
    double K_part2 = 2 * beta_invRho_invG;
    double K_part3 = (-alpha_invRho_invG +
                      K_part1);
    K =
        K_part3 /
        K_part2;
    // }
    // std::cout << "[shoot] K part1 = " << K_part1 << std::endl;
    // std::cout << "[shoot] K part2 = " << K_part2 << std::endl;
    // std::cout << "[shoot] K part3 = " << K_part3 << std::endl;
    // std::cout << "[shoot]  = " << K_part1 << std::endl;
    double dKdx_fenzi = s(0) - len,
           dKdx_fenmu = alpha_invRho_invG + 2 * beta_invRho_invG * K;
    dKdx_fenmu = std::max(0.0, dKdx_fenmu);
    double dKdx = (s[0] - len) / (dKdx_fenmu);
    // std::cout << "[pre] dKdx fenzi = " << dKdx_fenzi << std::endl;
    // std::cout << "[pre] dKdx fenmu = " << dKdx_fenmu << std::endl;
    // std::cout << "[pre] dKdx = " << dKdx << std::endl;
    // std::cout << "[pre] K = " << K << std::endl;
    // std::cout << "[shoot] alpha_invRho_invG = " << alpha_invRho_invG << std::endl;
    // std::cout << "[shoot] beta_invRho_invG = " << beta_invRho_invG << std::endl;

    double dydx = std::tan(theta0_deg * 3.1415926535 / 180.0);
    double ddydxds = -K * (1 + dydx * dydx);
    // std::cout << "[shoot] s = " << s.segment(0, 5).transpose() << std::endl;
    // std::cout << "[shoot] s(0) = " << s[0] << std::endl;
    // std::cout << "[shoot] alpha_invRho_invG + 2 * beta_invRho_invG * K = " << (alpha_invRho_invG + 2 * beta_invRho_invG * K) << std::endl;
    // std::cout << "[shoot] len = " << len << std::endl;

    // std::cout << "[shoot] M_invrho_invG = " << M_invrho_invG << std::endl;
    // std::cout << "[shoot] dKdx = " << dKdx << std::endl;
    // std::cout << "[shoot] dydx = " << dydx << std::endl;
    // std::cout << "[shoot] ddydxds = " << ddydxds << std::endl;
    // exit(1);
    for (int i = 0; i < numSamples - 1; i++)
    {
        double dx = ds / std::sqrt(1 + dydx * dydx);
        double dy = (dydx > 0 ? 1 : -1) * ds * std::sqrt(1 - 1.0 / (1 + dydx * dydx));

        // std::cout << "[step] " << i << " dx = " << dx << std::endl;
        // std::cout << "[step] " << i << " dy = " << dy << std::endl;

        x_solved(i + 1) = x_solved(i) + dx;
        y_solved(i + 1) = y_solved(i) + dy;

        // std::cout << "[step] " << i << " old K = " << K << std::endl;
        // std::cout << "[step] " << i << " before dKdx = " << dKdx << std::endl;
        // std::cout << "[step] " << i << " before dKdx * dx = " << dKdx * dx << std::endl;
        double new_K = K + dKdx * dx;
        // std::cout << "[step] " << i << " new K = " << new_K << std::endl;
        K = std::max(0.0, new_K);
        // std::cout << "[step] " << i << " clamped K = " << K << std::endl;
        double dKdx_fenzi = (s(i + 1) - len);
        double dKdx_fenmu = (alpha_invRho_invG + 2 * beta_invRho_invG * K);
        dKdx_fenmu = std::max(dKdx_fenmu, 1e-10);
        dKdx = dKdx_fenzi / dKdx_fenmu;

        dydx = dydx + ddydxds * ds;
        ddydxds = -K * (1 + dydx * dydx);

        // std::cout << "[step] " << i << " x = " << x_solved[i + 1] << std::endl;
        // std::cout << "[step] " << i << " y = " << y_solved[i + 1] << std::endl;
        // std::cout << "[step] " << i << " K = " << K << std::endl;
        // std::cout << "[step] " << i << " dKdx = " << dKdx << std::endl;
        // std::cout << "[step] " << i << " dydx = " << dydx << std::endl;
        // std::cout << "[step] " << i << " ddydxds = " << ddydxds << std::endl;
        // if (i > 5)
        //     exit(1);
    }
    // std::cout << "x = " << x_solved.transpose() << std::endl;
    // std::cout << "y = " << y_solved.transpose() << std::endl;
    double delta = x_solved.sum() * ds / len - len * guessXc;
    return delta;
}
#include "utils/LogUtil.h"
double cBezierShootSolver::ShootNonLinearSolveRobustNormalized(double rho_g_si, double nonlinear_2ndterm, double nonlinear_1stterm, double total_length_m, double theta0_rad, int num_of_samples, tVectorXd &x_solved,
                                                               tVectorXd &y_solved, bool select_k1)
{
    SIM_WARN("[warn] this method is easy to failed cuz of numericall error, when 2nd term is bigger than 1e-8");
    double guessXc = 0.25;
    double theta0_deg = theta0_rad / M_PI * 180;

    double alpha_inv_rho_invG = nonlinear_1stterm / (rho_g_si);
    double beta_inv_rho_invG = nonlinear_2ndterm / (rho_g_si);

    // std::cout << "[inner] alpha wenchao = " << alpha_inv_rho_invG << std::endl;
    // std::cout << "[inner] beta wenchao = " << beta_inv_rho_invG << std::endl;
    {
        // rho_g_si = 1;
        // beta_inv_rho_invG = 7.531909997462524e-11;
        // alpha_inv_rho_invG = 1.466397325207815e-05;
        // total_length_m = 0.0893474829655034;
        // theta0_deg = -35.4472317299905;
        // theta0_rad = theta0_deg / 180 * 3.1415;
    }

    double delta = NumericalIntegrate(theta0_deg, total_length_m, alpha_inv_rho_invG, beta_inv_rho_invG, num_of_samples, guessXc, x_solved, y_solved);
    // exit(1);
    double step = 0.25;
    double epslion = delta;
    int iter = 0;
    while (epslion != 0)
    {
        step = (delta > 0 ? 1 : -1) * std::fabs(step) / 2.0;
        guessXc = guessXc + step;

        double newDelta = NumericalIntegrate(theta0_deg, total_length_m, alpha_inv_rho_invG, beta_inv_rho_invG, num_of_samples, guessXc, x_solved, y_solved);
        if (iter % 10 == 0)
            std::cout << "[nonlinear] iter " << iter << " delta = " << newDelta << std::endl;
        epslion = newDelta - delta;
        delta = newDelta;
        iter += 1;
    }
    return delta;
    // while (epslion ~= 0)

    // end
}