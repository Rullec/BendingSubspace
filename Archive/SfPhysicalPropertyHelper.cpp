/**
* ��˾���������
* ��Ȩ��Ϣ�������������
* ������
* ���ߣ������
* ���ڣ�2021/04/15
*
*/

#include "SfPhysicalPropertyHelper.h"

#include "Geometry/SDGeometryMath.h"
#include "DataEngine/SDDataManager.h"

#include "BaseData/SfPhysicalProperty.h"
#include "BaseData/SfPhysicalPropertyMeasureData.h"
#include "BaseData/SfStretchesCalculater.h"

SF_USING_NAMESPACE
SD_USING_NAMESPACE

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


SfPhysicalPropertyPtr SfPhysicalPropertyHelper::GeneratePhysicalProperty(SfPhysicalPropertyMeasureDataPtr data)
{
	auto physicalProperty = SDDataManager::GetCurrent()->Create<SfPhysicalProperty>();
	
	auto StretchesData = data->GetStretchTest();
	SfLinearStretchesCalculater stretches;
	stretches.CalculateStretches(StretchesData);
	
	//!	����Bendingϵ��ת������
	SDMatrix3	factorMatrix;

	//!	�����Ƶ��İ汾: �ѷ���(��Ϊ��������ƫ��ϴ�ʱ��̫��)
	//factorMatrix(0, 0) = 2.0f * (28.0f - 13.0f * std::sqrt(2.0f));
	//factorMatrix(0, 1) = 2.0f * (6.0f * SDSCF(GTE_C_PI) - 13.0f * std::sqrt(2.0f));
	//factorMatrix(0, 2) = 2.0f * (-28.0f + 26.0f * std::sqrt(2.0f));
	// 
	//factorMatrix(1, 0) = 2.0f * (6.0f * SDSCF(GTE_C_PI) - 13.0f * std::sqrt(2.0f));
	//factorMatrix(1, 1) = 2.0f * (28.0f - 13.0f * std::sqrt(2.0f));
	//factorMatrix(1, 2) = 2.0f * (-28.0f + 26.0f * std::sqrt(2.0f));
	// 
	//factorMatrix(2, 0) = -(28.0f - 26.0f * std::sqrt(2));
	//factorMatrix(2, 1) = -(28.0f - 26.0f * std::sqrt(2));
	//factorMatrix(2, 2) = 2.0f * (28.0f - 26.0f * std::sqrt(2.0f)) + 12.0f * SDSCF(GTE_C_PI);
	//factorMatrix /= 9.0f * SDSCF(GTE_C_PI * GTE_C_PI);


	//!	ʵ�ʲ����İ汾
	factorMatrix(0, 0) = 0.216494632260111f;          // �����������ݵ���Matlab�϶�Ԫ���������
	factorMatrix(0, 1) = 0.0227527139889231f;         // ����һ�����Ի���ģ�����bending�����Զ�Ӧ����bendingǿ�ȹ���
	factorMatrix(0, 2) = 0.183236582216216f;

	factorMatrix(1, 0) = 0.0227527139889231f;
	factorMatrix(1, 1) = 0.216494632260111f;
	factorMatrix(1, 2) = 0.183236582216216f;

	factorMatrix(2, 0) = 0.091618291108108f;
	factorMatrix(2, 1) = 0.091618291108108f;
	factorMatrix(2, 2) = 0.239247346249034f;
	factorMatrix /= 50000.0f / 1.100f;		//!	����ϵ��alpha��ģ������֮���У��


	//!	�ֶ���С�����İ汾
	SDVector testValue0 = factorMatrix * SDVector(1.0f, 1.0f, 1.0f);
	SDVector diag = SDVector(factorMatrix(0, 0), factorMatrix(1, 1), factorMatrix(2, 2));
	factorMatrix(0, 0) = 0.0f;
	factorMatrix(1, 1) = 0.0f;
	factorMatrix(2, 2) = 0.0f;
	factorMatrix *= 0.1f;
	factorMatrix(0, 0) = diag[0];
	factorMatrix(1, 1) = diag[1];
	factorMatrix(2, 2) = diag[2];
	SDVector testValue1 = factorMatrix * SDVector(1.0f, 1.0f, 1.0f);
	factorMatrix *= (testValue0[0] / testValue1[0]);

	//!	���������ܶ�
	float rectArea = data->GetSize()[0] * data->GetSize()[1] * 1e-6f;	//!	��λ��ƽ����
	float density = (rectArea == 0.0f) ? 300.0f : data->GetWeight() / rectArea;	//!	��λ����

	//!	���㾭γб�Ĺ۲⿹��ǿ�ȣ����꣺GB/T 18318.1 -- 2009��
	constexpr float gravity = 9.8f;					//!	��ʽ����ʱʹ�õ����������� 9.8 m/s^2
	constexpr float stiffFactor = 1.0f / 8.0f;		//!	��Ť��Ϊ41.5��ʱ����ֵ���õ��Ľ����alpha = stiffFactor * rho * g * len^3.
	auto bendingData = data->GetBendingTest();
	float weftLength = bendingData.GetWeftLength() * 1e-3f;		//!	��λ����
	float warpLength = bendingData.GetWarpLength() * 1e-3f;		//!	��λ����
	float biasLength = bendingData.GetBiasLength() * 1e-3f;		//!	��λ����

	float ml = std::max(weftLength, warpLength);
	float ms = std::min(weftLength, warpLength);
	bool useNonlinear = ml / ms > 1.2f;

	float observedBendingWeft = density * gravity * (weftLength * weftLength * weftLength) * stiffFactor;
	float observedBendingWarp = density * gravity * (warpLength * warpLength * warpLength) * stiffFactor;
	float observedBendingBias = density * gravity * (biasLength * biasLength * biasLength) * stiffFactor;

	//!	��ʵ���ϵ�бɴǿ��һ����ھ���γǿ��֮�䡣
	observedBendingBias = SDGeometryMath::Clamp(observedBendingBias, std::min(observedBendingWeft, observedBendingWarp), std::max(observedBendingWeft, observedBendingWarp));

	//!		factorMatrix * bendingStiff = observeStiff;
	SDVector3 rhs(observedBendingWeft, observedBendingWarp, observedBendingBias);

	SDMatrix3	StS = SDGte::Transpose(factorMatrix) * factorMatrix;
	SDVector3	Strhs = SDGte::Transpose(factorMatrix) * rhs;

	double	bendingStiff[3] = { rhs[0],   rhs[1],  rhs[2] };
	double	b[3] = { Strhs[0],   Strhs[1],  Strhs[2] };

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
	if (useNonlinear)
		BendingNonLinearGaussSeidel(bendingStiff, mat, b);
	else
		BendingLinearGaussSeidel(bendingStiff, mat, b);

	//!	�������
	physicalProperty->SetWeight(density);
	physicalProperty->SetThickness(data->GetThickness());

	physicalProperty->SetBendingWeftRatio((float)std::clamp(bendingStiff[0], 0., 2000000.)); // Clamp ������ɵ�bending��Χ
	physicalProperty->SetBendingWarpRatio((float)std::clamp(bendingStiff[1], 0., 2000000.));
	physicalProperty->SetBendingBiasRatio((float)std::clamp(bendingStiff[2], 0., 2000000.));

	physicalProperty->SetStretchWeftRatio((float)std::clamp(stretches.m_weft, 0., 10000000.00));// Clamp ������ɵ����췶Χ
	physicalProperty->SetStretchWarpRatio((float)std::clamp(stretches.m_warp, 0., 10000000.00));
	physicalProperty->SetShearing((float)std::clamp(stretches.m_shearing, 0., 10000000.00));

	physicalProperty->SetHasMeasureData(true);
	physicalProperty->GetMeasureData()->CopyFrom(data);
	
	return physicalProperty;
}
