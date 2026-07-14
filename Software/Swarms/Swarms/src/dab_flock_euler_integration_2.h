/** \file dab_flock_euler_integration_h.h
 *
 *  Created by Daniel Bisig on 4/17/07.
 *  Ported to OpenFrameworks by Daniel Bisig on 12/08/17.
 */

#ifndef _dab_flock_euler_integration_2_h_
#define _dab_flock_euler_integration_2_h_

#include "dab_flock_behavior.h"
#include <Eigen/Dense>

namespace dab
{

    namespace flock
    {

        class EulerIntegration2 : public Behavior
        {
        public:
            /**
             \brief create behavior
             \param pInputParameterString input parameter string (parameters are space separated)
             \param pOutputParameterString output paramaters are space separated)
             */
            EulerIntegration2(const std::string& pInputParameterString, const std::string& pOutputParameterString);

            /**
             \brief create behavior
             \param pAgent agent this behavior belongs to
             \param pBehaviorName name of behavior
             \param pInputParameterString input parameter string (parameters are space separated)
             \param pOutputParameterString output paramaters are space separated)
             \exception Exception wrong number or type of parameters
             */
            EulerIntegration2(Agent* pAgent, const std::string& pBehaviorName, const std::string& pInputParameterString, const std::string& pOutputParameterString) throw (Exception);

            /**
             \brief destructor
             */
            ~EulerIntegration2();

            /**
             \brief create copy of behavior
             \param pAgent agent this behavior belongs to
             \param pBehaviorName name of behavior
             \return new behavior
             \exception Exception wrong number of type of parameters
             */
            virtual Behavior* create(const std::string& pBehaviorName, Agent* pAgent) const throw (Exception);

            /**
             \brief create copy of behavior
             \param pInputParameterString input parameter string
             \param pOutputParameterString output parameter string
             \return new behavior
             */
            virtual Behavior* create(const std::string& pInputParameterString, const std::string& pOutputParameterString) const;

            /**
             \brief perform integration
             */
            virtual void act();

        protected:
            Parameter* mDerivative0ParIn; /// \brief zero order derivative (input)
            Parameter* mDerivative1ParIn; /// \brief first order derivative (input)
            Parameter* mDerivative2ParIn; /// \brief second order derivative (input)
            Parameter* mDerivative0ParOut; /// \brief zero order derivative (output)
            Parameter* mDerivative1ParOut; /// \brief first order derivative (output)
            Parameter* mTimeStepPar; /// \brief integration time step (internal)
        };

    };

};

#endif