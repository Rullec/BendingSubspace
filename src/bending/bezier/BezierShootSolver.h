#pragma once
#include "BezierCurvePhysics.h"

class cBezierShootSolver
{
public:
    static double ShootLinearSolve(double beta, double theta0, double t0, double stepsize,
                                   tVectorXd &x_lst_solved,
                                   tVectorXd &y_lst_solved, bool silent = false);
    static double ShootNonLinearSolve(double m, double n, double theta0, double t0, double stepsize,
                                      tVectorXd &x_lst_solved,
                                      tVectorXd &y_lst_solved, bool silent = false);

    static double ShootLinearSolveWenchao(double beta, double theta0, int num_of_samples, tVectorXd &x_solved,
                                          tVectorXd &y_solved);
};