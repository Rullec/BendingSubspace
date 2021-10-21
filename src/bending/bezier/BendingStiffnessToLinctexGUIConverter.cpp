#include "BendingStiffnessToLinctexGUIConverter.h"
#include "utils/MathUtil.h"

tMatrix3d get_factor_matrix()
{

    tMatrix3d factor_matrix = tMatrix3d::Zero();

    factor_matrix(0, 0) = 0.216494632260111;  //  数据来自数据点在Matlab上多元非线性拟合
    factor_matrix(0, 1) = 0.0227527139889231; //  代表一阶线性化后的，界面bending参数对对应方向bending强度贡献
    factor_matrix(0, 2) = 0.183236582216216;

    factor_matrix(1, 0) = 0.0227527139889231;
    factor_matrix(1, 1) = 0.216494632260111;
    factor_matrix(1, 2) = 0.183236582216216;

    factor_matrix(2, 0) = 0.091618291108108;
    factor_matrix(2, 1) = 0.091618291108108;
    factor_matrix(2, 2) = 0.239247346249034;
    factor_matrix /= 50000.0 / 1.100; // !	理论系数alpha与模拟属性之间的校正

    // print(f"factor_matrix 1\n{factor_matrix}")

    tVector3d test_value0 = factor_matrix * tVector3d::Ones();
    // print(f"test value 0 {test_value0}")
    tVector3d diag = tVector3d(factor_matrix(0, 0), factor_matrix(1, 1), factor_matrix(2, 2));
    // print(f"diag {diag}")

    factor_matrix(0, 0) = 0;
    factor_matrix(1, 1) = 0;
    factor_matrix(2, 2) = 0;
    factor_matrix *= 0.1;
    factor_matrix(0, 0) = diag[0];
    factor_matrix(1, 1) = diag[1];
    factor_matrix(2, 2) = diag[2];
    // print(f"factor_matrix 2 \n {factor_matrix}")

    tVector3d test_value1 = factor_matrix * tVector3d::Ones();
    // print(f"test value 1 {test_value1}")
    factor_matrix *= (test_value0[0] / test_value1[1]);
    // print(f"factor_matrix 3 {factor_matrix}")

    return factor_matrix;
}

double use_nonlinear(double weft_length, double warp_length)
{

    double ml = std::max(weft_length, warp_length);
    double ms = std::min(weft_length, warp_length);
    return (ml / ms) > 1.2;
}

double get_bending_observed_stiffness(double density, double bending_length)
{
    double stiff_factor = 1.0 / 8;
    double gravity = 9.8;
    double stiffness = density * gravity * (std::pow(bending_length, 3)) * stiff_factor;
    return stiffness;
}

double bias_clamp(double weft, double warp, double bias)
{
    double min_val = std::min(weft, warp),
           max_val = std::max(weft, warp);
    bias = std::max(std::min(max_val, bias), min_val);
    return bias;
}

tMatrix3d get_GS_mat(const tMatrix3d &factor_mat)
{
    tMatrix3d StS = factor_mat.transpose() * factor_mat;
    return StS;
}

double convert_bending_simtogui(double value)
{
    std::vector<std::pair<double, double>> bending_map_guitosim = {{0e0, 0.0}, {1e2, 10.0}, {1e3, 28.0}, {3e3, 48.0}, {2e4, 65.0}, {2e5, 83.0}, {2e6, 100.0}};

    if (value < bending_map_guitosim[0].first)
        return bending_map_guitosim[0].second;
    for (int _idx = 0; _idx < bending_map_guitosim.size() - 1; _idx++)
    {
        auto map_st = bending_map_guitosim[_idx];
        double sim_st = map_st.first,
               gui_st = map_st.second;
        auto map_ed = bending_map_guitosim[_idx + 1];
        double sim_ed = map_ed.first,
               gui_ed = map_ed.second;
        if (sim_st <= value && value <= sim_ed)
        {
            return gui_st + (value - sim_st) / (sim_ed - sim_st) * (gui_ed - gui_st);
        }
    }
    if (value > bending_map_guitosim[bending_map_guitosim.size() - 1].first)
        return bending_map_guitosim[bending_map_guitosim.size() - 1].second;
}

tVector3d bending_nonlinear_gauss_seidel(const tMatrix3d &mat, const tVector3d &rhs)
{

    tVector3d out = tVector3d::Zero();
    for (int iter = 0; iter < 500; iter++)
    {
        int i = 0;
        for (i = 0; i < 2; i++)
        {

            double top = rhs[i];
            for (int j = 0; j < 3; j++)
            {
                if (i != j)
                    top -= mat(i, j) * out[j];
            }

            out[i] = top / mat(i, i);
            out[i] = std::max(out[i], 0.0);
        }
        i = 2;
        double top = rhs[i];
        double mi = std::min(out[1], out[0]);
        top -= mat(i, 0) * 2 * mi;
        out[i] = top / mat(i, i);
        out[i] = std::max(out[i], 0.0);
    }
    return out;
}

tVector3d bending_linear_gauss_seidel(const tMatrix3d &mat, const tVector3d &rhs)
{

    tVector3d out = tVector3d::Zero();
    for (int iter = 0; iter < 500; iter++)
    {

        for (int i = 0; i < 3; i++)
        {

            double top = rhs[i];
            for (int j = 0; j < 3; j++)
            {
                if (i != j)
                    top -= mat(i, j) * out[j];
            }
            out[i] = top / mat(i, i);
            out[i] = std::max(out[i], 0.0);
        }
        for (int i = 2; i >= 0; i--)
        {

            double top = rhs[i];
            for (int j = 0; j < 3; j++)
            {

                if (i != j)
                    top -= mat(i, j) * out[j];
            }
            out[i] = top / mat(i, i);
            out[i] = std::max(out[i], 0.0);
        }
    }
    return out;
}

tVector3d calculate_sim_param(double density, double warp_length, double weft_length, double bias_length)
{

    /*
    @density: [g.m^{-2}]
    @warp_length: [m]
    @weft_length: [m]
    @bias_length: [m]
    */
    tMatrix3d factor_matrix = get_factor_matrix();

    bool if_use_nonlinear = use_nonlinear(weft_length, warp_length);

    double obs_warp_length = get_bending_observed_stiffness(density, warp_length);
    double obs_weft_length = get_bending_observed_stiffness(density, weft_length);
    double obs_bias_length = get_bending_observed_stiffness(density, bias_length);
    obs_bias_length = bias_clamp(obs_weft_length, obs_warp_length,
                                 obs_bias_length);

    tVector3d rhs = tVector3d(obs_weft_length, obs_warp_length, obs_bias_length);

    tVector3d Strhs = factor_matrix * rhs;
    tVector3d b = Strhs;

    tMatrix3d mat = get_GS_mat(factor_matrix);
    tVector3d bending_stiff = tVector3d::Zero();
    if (if_use_nonlinear)
    {
        bending_stiff = bending_nonlinear_gauss_seidel(mat, b);
    }
    else
    {
        bending_stiff = bending_linear_gauss_seidel(mat, b);
    }
    return bending_stiff;
}

#include <iostream>

tVector3d cBendingStiffnessToLinctexGUIConverter::ConvertToLinctex_Linear(const tVector3d &bending_stiffness, double rho_g_si)
{
    // 1. bending stiffness to bending length
    // std::cout << "bending stiffness = " << bending_stiffness.transpose() << std::endl;
    // 2. bending length to bending stiffness sim
    // std::cout << "rho_g_si = " << rho_g_si << std::endl;
    double density = rho_g_si / 9.81; // kg / m^2
    tVector3d bending_length = 2 * (bending_stiffness / rho_g_si).array().pow(1.0 / 3);
    // std::cout << "bending_length = " << bending_length.transpose() << std::endl;
    tVector3d
        sim_value = calculate_sim_param(1e3 * density, bending_length[0], bending_length[1], bending_length[2]);
    tVector3d gui_value = tVector3d::Zero();
    gui_value[0] = convert_bending_simtogui(sim_value[0]);
    gui_value[1] = convert_bending_simtogui(sim_value[1]);
    gui_value[2] = convert_bending_simtogui(sim_value[2]);
    // std::cout << "gui_value = " << gui_value.transpose() << std::endl;
    // exit(0);
    return gui_value;

    // 3. bending stiffness sim to GUI value
}

tVectorXd cBendingStiffnessToLinctexGUIConverter::ConvertToLinctex_NonLinear(const tVector3d &bending_stiffness_1stterm, const tVector3d &bending_stiffness_2ndterm, double rho_g_si)
{
    tVector3d bs_1st_gui = cBendingStiffnessToLinctexGUIConverter::ConvertToLinctex_Linear(bending_stiffness_1stterm, rho_g_si);
    // tVector3d bs_2nd_gui = cBendingStiffnessToLinctexGUIConverter::ConvertToLinctex_Linear(bending_stiffness_2ndterm, rho_g_si);
    tVector3d sign = tVector3d::Zero();
    tVector3d bs_2nd_raw = bending_stiffness_2ndterm;
    // record the symbol, and convert them to positive
    for (int i = 0; i < 3; i++)
    {
        sign[i] = bending_stiffness_2ndterm[i] > 0 ? 1 : -1;
        bs_2nd_raw[i] *= sign[i] * 30;
    }
    tVector3d bs_2nd_gui = sign.cwiseProduct(cBendingStiffnessToLinctexGUIConverter::ConvertToLinctex_Linear(bs_2nd_raw, rho_g_si));
    tVectorXd res = tVectorXd::Zero(6);
    res.segment(0, 3) = bs_1st_gui;
    res.segment(3, 3) = bs_2nd_gui;
    return res;
}