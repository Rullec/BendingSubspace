#include <iostream>
#include <Eigen/Dense>
using namespace Eigen;

typedef Eigen::VectorXd tVectorXd;
typedef Eigen::MatrixXd tMatrixXd;
typedef Eigen::Matrix3d tMatrix3d;
typedef Eigen::Vector3d tVector3d;

static void BendingNonLinearGaussSeidel(double out[3], const double mat[3][3], const double rhs[3])
{
    for (int iter = 0; iter < 500; iter++)
    {
        for (int i = 0; i < 2; i++)
        {
            double top = rhs[i];
            for (int j = 0; j < 3; j++)
            {
                if (i != j)
                    top -= mat[i][j] * out[j];
            }

            out[i] = top / mat[i][i];
            out[i] = std::max(out[i], 0.);
        }
        {
            int i = 2;
            double top = rhs[i];
            double mi = std::min(out[1], out[0]);
            top -= mat[i][0] * 2 * mi;
            out[i] = top / mat[i][i];
            out[i] = std::max(out[i], 0.);
        }
    }
}

static void BendingLinearGaussSeidel(double out[3], const double mat[3][3], const double rhs[3])
{
    for (int iter = 0; iter < 500; iter++)
    {
        for (int i = 0; i < 3; i++)
        {
            double top = rhs[i];
            for (int j = 0; j < 3; j++)
            {
                if (i != j)
                    top -= mat[i][j] * out[j];
            }

            out[i] = top / mat[i][i];
            out[i] = std::max(out[i], 0.);
        }

        for (int i = 2; i >= 0; i--)
        {
            double top = rhs[i];
            for (int j = 0; j < 3; j++)
            {
                if (i != j)
                    top -= mat[i][j] * out[j];
            }

            out[i] = top / mat[i][i];
            out[i] = std::max(out[i], 0.);
        }
    }
}

tMatrix3d GetFactorMat()
{
    //!	计算Bending系数转换矩阵
    tMatrix3d factorMatrix;

    //!	实际测量的版本
    factorMatrix(0, 0) = 0.216494632260111f;  // 数据来自数据点在Matlab上多元非线性拟合
    factorMatrix(0, 1) = 0.0227527139889231f; // 代表一阶线性化后的，界面bending参数对对应方向bending强度贡献
    factorMatrix(0, 2) = 0.183236582216216f;

    factorMatrix(1, 0) = 0.0227527139889231f;
    factorMatrix(1, 1) = 0.216494632260111f;
    factorMatrix(1, 2) = 0.183236582216216f;

    factorMatrix(2, 0) = 0.091618291108108f;
    factorMatrix(2, 1) = 0.091618291108108f;
    factorMatrix(2, 2) = 0.239247346249034f;
    factorMatrix /= 50000.0f / 1.100f; //!	理论系数alpha与模拟属性之间的校正

    //!	手动减小耦合项的版本
    tVector3d testValue0 = factorMatrix * tVector3d(1.0f, 1.0f, 1.0f);
    tVector3d diag = tVector3d(factorMatrix(0, 0), factorMatrix(1, 1), factorMatrix(2, 2));
    factorMatrix(0, 0) = 0.0f;
    factorMatrix(1, 1) = 0.0f;
    factorMatrix(2, 2) = 0.0f;
    factorMatrix *= 0.1f;
    factorMatrix(0, 0) = diag[0];
    factorMatrix(1, 1) = diag[1];
    factorMatrix(2, 2) = diag[2];
    tVector3d testValue1 = factorMatrix * tVector3d(1.0f, 1.0f, 1.0f);
    factorMatrix *= (testValue0[0] / testValue1[0]);
    return factorMatrix;
}

// SfPhysicalPropertyPtr SfPhysicalPropertyHelper::GeneratePhysicalProperty(SfPhysicalPropertyMeasureDataPtr data)
// {
//     // auto physicalProperty = SDDataManager::GetCurrent()->Create<SfPhysicalProperty>();

//     // auto StretchesData = data->GetStretchTest();
//     // SfLinearStretchesCalculater stretches;
//     // stretches.CalculateStretches(StretchesData);

//     // //!	计算质量密度
//     // float rectArea = data->GetSize()[0] * data->GetSize()[1] * 1e-6f;           //!	单位：平方米
//     // float density = (rectArea == 0.0f) ? 300.0f : data->GetWeight() / rectArea; //!	单位：克

//     // //!	计算经纬斜的观测抗弯强度（国标：GB/T 18318.1 -- 2009）
//     // constexpr float gravity = 9.8f;            //!	公式对齐时使用的重力加速是 9.8 m/s^2
//     // constexpr float stiffFactor = 1.0f / 8.0f; //!	当扭角为41.5度时，数值求解得到的结果，alpha = stiffFactor * rho * g * len^3.
//     // auto bendingData = data->GetBendingTest();
//     // float weftLength = bendingData.GetWeftLength() * 1e-3f; //!	单位：米
//     // float warpLength = bendingData.GetWarpLength() * 1e-3f; //!	单位：米
//     // float biasLength = bendingData.GetBiasLength() * 1e-3f; //!	单位：米

//     // float ml = std::max(weftLength, warpLength);
//     // float ms = std::min(weftLength, warpLength);
//     // bool useNonlinear = ml / ms > 1.2f;

//     // float observedBendingWeft = density * gravity * (weftLength * weftLength * weftLength) * stiffFactor;
//     // float observedBendingWarp = density * gravity * (warpLength * warpLength * warpLength) * stiffFactor;
//     // float observedBendingBias = density * gravity * (biasLength * biasLength * biasLength) * stiffFactor;

//     // //!	真实布料的斜纱强度一般界于经、纬强度之间。
//     // observedBendingBias = SDGeometryMath::Clamp(observedBendingBias, std::min(observedBendingWeft, observedBendingWarp), std::max(observedBendingWeft, observedBendingWarp));

//     //!	填充数据
//     physicalProperty->SetWeight(density);
//     physicalProperty->SetThickness(data->GetThickness());

//     physicalProperty->SetBendingWeftRatio((float)std::clamp(bendingStiff[0], 0., 2000000.)); // Clamp 到界面可调bending范围
//     physicalProperty->SetBendingWarpRatio((float)std::clamp(bendingStiff[1], 0., 2000000.));
//     physicalProperty->SetBendingBiasRatio((float)std::clamp(bendingStiff[2], 0., 2000000.));

//     physicalProperty->SetStretchWeftRatio((float)std::clamp(stretches.m_weft, 0., 10000000.00)); // Clamp 到界面可调拉伸范围
//     physicalProperty->SetStretchWarpRatio((float)std::clamp(stretches.m_warp, 0., 10000000.00));
//     physicalProperty->SetShearing((float)std::clamp(stretches.m_shearing, 0., 10000000.00));

//     physicalProperty->SetHasMeasureData(true);
//     physicalProperty->GetMeasureData()->CopyFrom(data);

//     return physicalProperty;
// }

int main()
{
    full_data = 19.067285308890607
    obs =  
    double observedBendingWeft = 19.067285308890607 * 1e-2 * \rho * g; // m
    double observedBendingWarp = 8.080198791573778 * 1e-2;
    double observedBendingBias = 11.332463933974608 * 1e-2;
    tMatrix3d factor_mat = GetFactorMat();
    //!		factorMatrix * bendingStiff = observeStiff;
    tVector3d rhs = tVector3d(observedBendingWeft, observedBendingWarp, observedBendingBias);

    tMatrix3d StS = factor_mat.transpose() * factor_mat;
    tVector3d Strhs = factor_mat.transpose() * rhs;

    double bendingStiff[3] = {rhs[0], rhs[1], rhs[2]};
    double b[3] = {Strhs[0], Strhs[1], Strhs[2]};

    double mat[3][3];
    mat[0][0] = StS(0, 0);
    mat[0][1] = StS(0, 1);
    mat[0][2] = StS(0, 2);

    mat[1][0] = StS(1, 0);
    mat[1][1] = StS(1, 1);
    mat[1][2] = StS(1, 2);

    mat[2][0] = StS(2, 0);
    mat[2][1] = StS(2, 1);
    mat[2][2] = StS(2, 2);
    BendingNonLinearGaussSeidel(bendingStiff, mat, b);

    std::cout << bendingStiff[0] << " " << bendingStiff[1] << " " << bendingStiff[2] << std::endl;
}