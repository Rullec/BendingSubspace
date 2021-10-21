#pragma once
#include "utils/MathUtil.h"
/**
 * \brief       convert the bending stiffness to Linctex GUI simulation value
*/
class cBendingStiffnessToLinctexGUIConverter
{
public:
    static tVector3d ConvertToLinctex_Linear(const tVector3d &bending_stiffness, double rho_g_si);
    static tVectorXd ConvertToLinctex_NonLinear(const tVector3d &bending_stiffness_1stterm, const tVector3d &bending_stiffness_2ndterm, double rho_g_si);
};