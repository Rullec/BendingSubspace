#include "BezierCurve.h"
#include "utils/DefUtil.h"
#include "utils/LogUtil.h"
#include <iostream>
tVectorXd GetLinspace(float low, float high, int num_of_points)
{
    SIM_ASSERT(num_of_points > 1);
    float step = (high - low) / (num_of_points - 1);
    tVectorXd res = tVectorXd::Zero(num_of_points);
    for (int i = 0; i < num_of_points; i++)
    {
        res[i] = i * step;
    }
    return res;
}

// extern void CalcEdgeDrawBufferSingle(const tVector &v0, const tVector &v1,
//                                      const tVector &edge_normal,
//                                      Eigen::Map<tVectorXf> &buffer, int &st_pos,
//                                      const tVector3f &color);

cBezierCurve::cBezierCurve(int num_of_div, const tVector2d &A,
                           const tVector2d &B, const tVector2d &C,
                           const tVector2d &D)
    : A(A), B(B), C(C), D(D), mNumOfDiv(num_of_div)
{
    // init the edge buffer
    InitPointlist(mPointList);
    mPosX = mPointList.row(0);
    mPosY = mPointList.row(1);
    mCurvatureList = CalculateCurvature();
    mArclengthList = CalculateArcLengthList();
    // std::cout << "arclength = " << mArclengthList.transpose() << std::endl;
    // exit(1);
    // printf("point shape %d, %d\n", mPointList.rows(), mPointList.cols());
    // mPointList.row(0) = GetLinspace(0, 1, mNumOfDiv);
    // mPointList.row(1) = GetLinspace(0, 1, mNumOfDiv);

    // int st = 0;
    // int num_of_edges = GetNumOfDrawEdges();
    // int num_of_vertices = num_of_edges * 2;
    // int buffer_size = num_of_vertices * RENDERING_SIZE_PER_VERTICE;
    // mDrawBuffer.noalias() = tVectorXf::Zero(buffer_size);
    // Eigen::Map<tVectorXf> map(mDrawBuffer.data(), mDrawBuffer.size());
    // for (int i = 0; i < mNumOfDiv - 1; i++)
    // {
    //     tVector st_pos = tVector(0, mPointList(0, i), mPointList(1, i), 0);
    //     tVector ed_pos =
    //         tVector(0, mPointList(0, i + 1), mPointList(1, i + 1), 0);
    //     CalcEdgeDrawBufferSingle(st_pos, ed_pos, tVector::Zero(), map, st,
    //                              tVector3f::Zero());
    // }
}

int cBezierCurve::GetNumOfDrawEdges() const { return mNumOfDiv - 1; }

// const tVectorXf &cBezierCurve::GetDrawBuffer() { return mDrawBuffer; }

int factorial(int n)
{
    if (n == 0)
        return 1;
    int val = 1;
    for (int i = 1; i <= n; i++)
        val *= i;
    return val;
}

int Com(int n, int k)
{
    SIM_ASSERT((n >= 1) && ((k >= 0) && (k <= n)));
    return int(factorial(n) / (factorial(k) * factorial(n - k)));
}

void cBezierCurve::InitPointlist(tMatrixXd &point_lst)
{
    int order = 3;
    mU = GetLinspace(0, 1, mNumOfDiv);
    // std::cout << "u = " << u.size() << std::endl;
    mOneMinusU = tVectorXd::Ones(mNumOfDiv) - mU;
    // std::cout << "1 - u = " << one_minus_u.size() << std::endl;
    point_lst.noalias() = tMatrixXd::Zero(2, mNumOfDiv);
    tMatrixXd ctrl_point_lst = tMatrixXd::Zero(2, 4);
    ctrl_point_lst.col(0) = A;
    ctrl_point_lst.col(1) = B;
    ctrl_point_lst.col(2) = C;
    ctrl_point_lst.col(3) = D;
    for (int i = 0; i <= order; i++)
    {
        // (N \times 1)
        tVectorXd res =
            (mU.array().pow(i) * mOneMinusU.array().pow(order - i)).transpose();
        tVectorXd ctrl_pt = ctrl_point_lst.col(i);
        tMatrixXd new_point_incre = Com(order, i) * ctrl_pt * res.transpose();
        point_lst.noalias() += new_point_incre;
    }
}

double cBezierCurve::GetTotalLength() const
{
    double total_length = 0;
    for (int i = 0; i < this->mPointList.cols() - 1; i++)
    {
        total_length += (mPointList.col(i + 1) - mPointList.col(i)).norm();
    }
    return total_length;
}

tEigenArr<tVector2d> cBezierCurve::GetPointList()
{
    tEigenArr<tVector2d> pt_lst(0);
    for (int i = 0; i < this->mPointList.cols(); i++)
    {
        pt_lst.push_back(mPointList.col(i));
    }
    return pt_lst;
}

/**
 * \brief           K(t) = |r'(t) * r''(t)| / (|r'(t)^3|)
 * 
 * r'(t) = drdt = 
 *              = 3 * {
 *                 [3 * (B - C) + D - A] t^2 
 *                 + 2 ( A- 2 B + C) t
 *                 - A + B
 *              }
 * 
 *          r''(t)
 *              = drdt 
 *              = 6 *{
 *                  [3 (B - C) + D - A]t 
 *                  + A - 2 B + C
 *                  }
*/
tVectorXd cross2d(
    const tMatrixXd &a,
    const tMatrixXd &b)
{
    SIM_ASSERT(a.rows() == 2);
    SIM_ASSERT(b.rows() == 2);
    int num = a.cols();

    tVectorXd res(num);
    for (int i = 0; i < num; i++)
    {
        tVector3d a_vec = tVector3d(a.col(i)[0], a.col(i)[1], 0),
                  b_vec = tVector3d(b.col(i)[0], b.col(i)[1], 0);
        // std::cout << "a vec = " << a_vec.transpose() << std::endl;
        // std::cout << "b vec = " << b_vec.transpose() << std::endl;
        res[i] = std::fabs((a_vec.cross(b_vec))[2]);
    }
    return res;
}
tVectorXd cBezierCurve::CalculateCurvature() const
{
    // std::cout << "A = " << A.transpose() << std::endl;
    // std::cout << "B = " << B.transpose() << std::endl;
    // std::cout << "C = " << C.transpose() << std::endl;
    // std::cout << "D = " << D.transpose() << std::endl;
    tVectorXd u2 = this->mU.cwiseProduct(mU);
    // std::cout << "mU = " << mU.transpose() << std::endl;
    tMatrixXd r_prime = (3 * (B - C) + D - A) * u2.transpose() + 2 * (A - 2 * B + C) * mU.transpose();
    // std::cout << "r_prime begin = \n"
    //           << r_prime << std::endl;
    r_prime.colwise() += -A + B;
    r_prime *= 3;
    // std::cout << "r_prime end = \n"
    //           << r_prime << std::endl;

    tMatrixXd r_prime2 = (3 * (B - C) + D - A) * mU.transpose();
    r_prime2.colwise() += A - 2 * B + C;
    r_prime2 *= 6;
    // std::cout << "r_prime2 = \n"
    //           << r_prime2 << std::endl;

    tVectorXd r_prime_pow3 = r_prime.colwise().norm(); // (1, N)
    r_prime_pow3 = r_prime_pow3.array().pow(3);
    // std::cout << "r_prime_pow3 = " << r_prime_pow3.transpose() << std::endl;
    tVectorXd fenzi = cross2d(r_prime, r_prime2); // (2, N)
    // std::cout << "fenzi = "
    //           << fenzi.transpose() << std::endl;
    // std::cout << "fenzi = " << fenzi.transpose() << std::endl;
    // std::cout << "fenmu = " << r_prime_pow3.transpose() << std::endl;
    tVectorXd K_lst = fenzi.cwiseQuotient(r_prime_pow3);
    // std::cout << "K = " << K_lst.transpose() << std::endl;
    // exit(1);
    return K_lst;
}

tVectorXd cBezierCurve::CalculateArcLengthList() const
{
    int num_of_seg = mNumOfDiv - 1;
    tVectorXd arc_length_lst = (mPointList.block(0, 0, 2, num_of_seg) - mPointList.block(0, 1, 2, num_of_seg)).colwise().norm();
    return arc_length_lst;
}