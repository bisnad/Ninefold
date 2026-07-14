/** \file dab_flock_euler_integration_2.cpp
 */

#include "dab_flock_euler_integration_2.h"
#include "dab_flock_parameter.h"
#include "dab_flock_agent.h"

using namespace dab;
using namespace dab::flock;

EulerIntegration2::EulerIntegration2(const std::string& pInputParameterString, const std::string& pOutputParameterString)
	: Behavior(pInputParameterString, pOutputParameterString)
{
	mClassName = "EulerIntegration2";
}

EulerIntegration2::EulerIntegration2(Agent* pAgent, const std::string& pBehaviorName, const std::string& pInputParameterString, const std::string& pOutputParameterString) throw (Exception)
	: Behavior(pAgent, pBehaviorName, pInputParameterString, pOutputParameterString)
{
	mClassName = "EulerIntegration2";

	if (mInputParameters.size() < 3) throw Exception("FLOCK ERROR: Behavior " + pBehaviorName + " : " + std::to_string(mOutputParameters.size()) + " Input Parameters Supplied, 3 Needed", __FILE__, __FUNCTION__, __LINE__);
	if (mOutputParameters.size() < 2) throw Exception("FLOCK ERROR: Behavior " + pBehaviorName + " : " + std::to_string(mOutputParameters.size()) + " Output Parameters Supplied, 2 Needed", __FILE__, __FUNCTION__, __LINE__);
	if (mInputParameters[0]->dim() != mOutputParameters[0]->dim()) throw Exception("FLOCK ERROR: Behavior " + pBehaviorName + " : input parameter " + mInputParameters[0]->name() + " dim " + std::to_string(mInputParameters[0]->dim()) + " must match output parameter " + mOutputParameters[0]->name() + " dim " + std::to_string(mOutputParameters[0]->dim()), __FILE__, __FUNCTION__, __LINE__);
	if (mInputParameters[1]->dim() != mOutputParameters[0]->dim())  throw Exception("FLOCK ERROR: Behavior " + pBehaviorName + " : input parameter " + mInputParameters[1]->name() + " dim " + std::to_string(mInputParameters[1]->dim()) + " must match output parameter " + mOutputParameters[0]->name() + " dim " + std::to_string(mOutputParameters[0]->dim()), __FILE__, __FUNCTION__, __LINE__);
	if (mInputParameters[2]->dim() != mOutputParameters[1]->dim())  throw Exception("FLOCK ERROR: Behavior " + pBehaviorName + " : input parameter " + mInputParameters[2]->name() + " dim " + std::to_string(mInputParameters[2]->dim()) + " must match output parameter " + mOutputParameters[0]->name() + " dim " + std::to_string(mOutputParameters[0]->dim()), __FILE__, __FUNCTION__, __LINE__);
	if (mOutputParameters[0]->dim() != mOutputParameters[1]->dim())  throw Exception("FLOCK ERROR: Behavior " + pBehaviorName + " : output parameter " + mOutputParameters[0]->name() + " dim " + std::to_string(mOutputParameters[0]->dim()) + " must match output parameter " + mOutputParameters[1]->name() + " dim " + std::to_string(mOutputParameters[1]->dim()), __FILE__, __FUNCTION__, __LINE__);


	// input parameter
	mDerivative0ParIn = mInputParameters[0];
	mDerivative1ParIn = mInputParameters[1];
	mDerivative2ParIn = mInputParameters[2];

	// output parameter
	mDerivative0ParOut = mOutputParameters[0];
	mDerivative1ParOut = mOutputParameters[1];

	// internal parameter
	mTimeStepPar = createInternalParameter("timestep", mDerivative0ParIn->dim(), 0.1);
}

EulerIntegration2::~EulerIntegration2()
{}

Behavior*
EulerIntegration2::create(const std::string& pBehaviorName, Agent* pAgent) const throw (Exception)
{
	try
	{
		return new EulerIntegration2(pAgent, pBehaviorName, mInputParameterString, mOutputParameterString);
	}
	catch (Exception& e)
	{
		throw;
	}
}

Behavior*
EulerIntegration2::create(const std::string& pInputParameterString, const std::string& pOutputParameterString) const
{
	try
	{
		return new EulerIntegration2(pInputParameterString, pOutputParameterString);
	}
	catch (Exception& e)
	{
		throw;
	}
}

void
EulerIntegration2::act()
{
	Eigen::VectorXf& derivative0In = mDerivative0ParIn->values();
	Eigen::VectorXf& derivative1In = mDerivative1ParIn->values();
	Eigen::VectorXf& derivative2In = mDerivative2ParIn->values();
	Eigen::VectorXf& derivative0Out = mDerivative0ParOut->backupValues();
	Eigen::VectorXf& derivative1Out = mDerivative1ParOut->backupValues();
	Eigen::VectorXf& timeStep = mTimeStepPar->values();

	derivative0Out += derivative1In.cwiseProduct(timeStep);
	derivative1Out += derivative2In.cwiseProduct(timeStep);
}