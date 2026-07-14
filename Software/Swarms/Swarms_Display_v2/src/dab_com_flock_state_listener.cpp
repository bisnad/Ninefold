#include "dab_com_flock_state_listener.hpp"

using namespace dab;
using namespace dab::flock;

FlockStateListener::FlockStateListener( dab::flock::Swarm* pPresetFlock, dab::flock::Swarm* pFlock, dab::space::SpaceGrid* pSpaceGrid)
    : mPresetFlock(pPresetFlock)
    , mFlock(pFlock)
    , mSpaceGrid(pSpaceGrid)
{
    std::cout << "mSpaceGrid " << mSpaceGrid << "\n";
    
    //updateSpaceGrid();
}

void
FlockStateListener::notify( std::shared_ptr<dab::OscMessage> pMessage )
{
    //std::cout << pMessage->address() << "\n";

    const std::string& messageAddress = pMessage->address();
    const std::vector< _OscArg* >& messageArguments = pMessage->arguments();
    
    if(messageAddress.find("/swarm") != std::string::npos && messageAddress.find("/position") != std::string::npos)
    {
        setFlockPosition(pMessage->arguments());
    }
    else if(messageAddress.find("/swarm") != std::string::npos && messageAddress.find("/velocity") != std::string::npos)
    {
        setFlockVelocity(pMessage->arguments());
    }
    else if(messageAddress.find("/preset_swarm") != std::string::npos && messageAddress.find("/position") != std::string::npos)
    {
        setPresetFlockPosition(pMessage->arguments());
    }
    else if(messageAddress.find("/preset_swarm") != std::string::npos && messageAddress.find("/velocity") != std::string::npos)
    {
        setPresetFlockVelocity(pMessage->arguments());
    }
}

void
FlockStateListener::setPresetFlockPosition(const std::vector< dab::_OscArg* >& pOscArguments)
{
    bool presetFlockChanged = false;
    int argCount = pOscArguments.size();
    
    int posParIndex = mPresetFlock->parameterIndex("position");
    std::vector<dab::flock::Agent*>& agents = mPresetFlock->agents();
    std::array<float, 3> pos;
    
    for(int argI=0, agentI=0; argI < argCount; argI += 3, agentI += 1)
    {
        dab::flock::Parameter* posPar = agents[agentI]->parameter(posParIndex);
        
        pos[0] = *(pOscArguments[argI]);
        pos[1] = *(pOscArguments[argI + 1]);
        pos[2] = *(pOscArguments[argI + 2]);
        
        Eigen::VectorXf& posParValuesOrig = posPar->values();

        for(int d=0; d<3; ++d)
        {
            if(posParValuesOrig[d] != pos[d]) presetFlockChanged = true;
        }

        posPar->setValues(3, pos.data());
        
        //std::cout << "agentI " << agentI << " pos " << pos[0]  << " " << pos[1] << " " << pos[2] << "\n";
    }
   
    //std::cout << "presetFlockChanged " << presetFlockChanged << "\n";

    if(presetFlockChanged == true) updateSpaceGrid();
}

void
FlockStateListener::setPresetFlockVelocity(const std::vector< dab::_OscArg* >& pOscArguments)
{
    int argCount = pOscArguments.size();
    
    int velParIndex = mPresetFlock->parameterIndex("velocity");
    std::vector<dab::flock::Agent*>& agents = mPresetFlock->agents();
    std::array<float, 3> vel;
    
    for(int argI=0, agentI=0; argI < argCount; argI += 3, agentI += 1)
    {
        dab::flock::Parameter* velPar = agents[agentI]->parameter(velParIndex);
        
        vel[0] = *(pOscArguments[argI]);
        vel[1] = *(pOscArguments[argI + 1]);
        vel[2] = *(pOscArguments[argI + 2]);
        
        velPar->setValues(3, vel.data());
        
        //std::cout << "agentI " << agentI << " pos " << vel[0] << " " << vel[1] << " " << vel[2] << "\n";
    }
}

void
FlockStateListener::setFlockPosition(const std::vector< dab::_OscArg* >& pOscArguments)
{
    int argCount = pOscArguments.size();
    
    int posParIndex = mFlock->parameterIndex("position");
    std::vector<dab::flock::Agent*>& agents = mFlock->agents();
    std::array<float, 3> pos;
    
    for(int argI=0, agentI=0; argI < argCount; argI += 3, agentI += 1)
    {
        dab::flock::Parameter* posPar = agents[agentI]->parameter(posParIndex);
        
        pos[0] = *(pOscArguments[argI]);
        pos[1] = *(pOscArguments[argI + 1]);
        pos[2] = *(pOscArguments[argI + 2]);
        
        posPar->setValues(3, pos.data());
        
        //std::cout << "agentI " << agentI << " pos " << pos[0]  << " " << pos[1] << " " << pos[2] << "\n";
    }
}

void
FlockStateListener::setFlockVelocity(const std::vector< dab::_OscArg* >& pOscArguments)
{
    int argCount = pOscArguments.size();
    
    int velParIndex = mFlock->parameterIndex("velocity");
    std::vector<dab::flock::Agent*>& agents = mFlock->agents();
    std::array<float, 3> vel;
    
    for(int argI=0, agentI=0; argI < argCount; argI += 3, agentI += 1)
    {
        dab::flock::Parameter* velPar = agents[agentI]->parameter(velParIndex);
        
        vel[0] = *(pOscArguments[argI]);
        vel[1] = *(pOscArguments[argI + 1]);
        vel[2] = *(pOscArguments[argI + 2]);
        
        velPar->setValues(3, vel.data());
        
        //std::cout << "agentI " << agentI << " pos " << vel[0] << " " << vel[1] << " " << vel[2] << "\n";
    }
}

void
FlockStateListener::updateSpaceGrid()
{

    int presetPosParIndex = mPresetFlock->parameterIndex("position");
    std::vector<dab::flock::Agent*>& presetAgents = mPresetFlock->agents();
    
    const Eigen::VectorXf& fieldMinPos = mSpaceGrid->minPos();
    const Eigen::VectorXf& fieldMaxPos = mSpaceGrid->maxPos();
    Eigen::VectorXf fieldSize = fieldMaxPos -fieldMinPos;
    
    math::VectorField<float>& vectorField = mSpaceGrid->vectorField();
    std::vector< Eigen::VectorXf >& fieldVectors = vectorField.vectors();
    const dab::Array<unsigned int>& fieldCount = mSpaceGrid->subdivisionCount();
    Eigen::VectorXf fieldPosIncr(3);
    fieldPosIncr << fieldSize[0] / fieldCount[0], fieldSize[1] / fieldCount[1], fieldSize[2] / fieldCount[2];
    
    //std::cout << "fieldMinPos " << fieldMinPos << "\n";
    //std::cout << "fieldMaxPos " << fieldMaxPos << "\n";
    //std::cout << "fieldSize " << fieldSize << "\n";
    //std::cout << "fieldCount " << fieldCount << "\n";
    //std::cout << "fieldPosIncr " << fieldPosIncr << "\n";
    

    Eigen::VectorXf fieldVecPos(3);
    float maxPosDiff = 1.0;
    
    fieldVecPos[2] = fieldMinPos[2];
    
    for(int z=0, i=0; z<fieldCount[2]; ++z)
    {
        fieldVecPos[2] += fieldPosIncr[2];
        fieldVecPos[1] = fieldMinPos[1];
        
        for(int y=0; y<fieldCount[1]; ++y)
        {

            fieldVecPos[1] += fieldPosIncr[1];
            fieldVecPos[0] = fieldMinPos[0];
            
            for(int x=0; x<fieldCount[0]; ++x, ++i)
            {
                fieldVecPos[0] += fieldPosIncr[0];
                
                // gather all position differences between current field vector and all preset positions
                std::vector<Eigen::VectorXf> fieldPresetPosDiffs;
                std::vector<float> fieldPresetPosDistances;
                float totalFieldPresetPosDistance = 0.0;

                for(int aI=0; aI<presetAgents.size(); ++aI)
                {
                    //std::cout << "aI " << aI << "\n";
                    
                    dab::flock::Parameter* presetPosPar = presetAgents[aI]->parameter(presetPosParIndex);
                    Eigen::VectorXf& presetPos = presetPosPar->values();
                    Eigen::VectorXf fieldPresetPosDiff = presetPos - fieldVecPos;
                    float fieldPresetPosDist = fieldPresetPosDiff.norm();
                    
                    fieldPresetPosDiffs.push_back(fieldPresetPosDiff);
                    fieldPresetPosDistances.push_back(fieldPresetPosDist);
                    
                    totalFieldPresetPosDistance += fieldPresetPosDist;
                }

                // 1) Compute unnormalized weights that *decrease* with distance
                std::vector<float> weights(presetAgents.size());
                float weightSum = 0.0f;
                const float eps = 1e-6f;
                const float p = 2.0f; // exponent, tweak as needed

                for (int aI = 0; aI < presetAgents.size(); ++aI) {
                    float d = fieldPresetPosDistances[aI];
                    // ensure non-zero distance
                    float w = 1.0f / std::pow(d + eps, p);
                    weights[aI] = w;
                    weightSum += w;
                }

                // 2) Build the direction as weighted sum of normalized direction vectors
                fieldVectors[i] = Eigen::Vector3f(0.0f, 0.0f, 0.0f);

                for (int aI = 0; aI < presetAgents.size(); ++aI) {
                    Eigen::VectorXf dir = fieldPresetPosDiffs[aI] / (fieldPresetPosDistances[aI] + eps);
                    float weightNorm = weights[aI] / weightSum; // sum of all weightNorm = 1
                    fieldVectors[i] += dir.cast<float>() * weightNorm;
                }

                // 3) Optionally normalize the result to ensure max length 1
                float len = fieldVectors[i].norm();
                if (len > 1e-6f) {
                    fieldVectors[i] /= len;
                }

                /*
                // calculate field vector directions
                fieldVectors[i] = Eigen::Vector3f( 0.0f, 0.0f, 0.0f );
                
                for(int aI=0; aI<presetAgents.size(); ++aI)
                {
                    Eigen::VectorXf fieldPresetPosDiffNorm = fieldPresetPosDiffs[aI] / (fieldPresetPosDistances[aI] + 0.000001);
                    //float fieldPresetPosDiffScale = 1.0 - (fieldPresetPosDistances[aI] / totalFieldPresetPosDistance);
                    float fieldPresetPosDiffScale = fieldPresetPosDistances[aI] / totalFieldPresetPosDistance;

                    fieldVectors[i] += fieldPresetPosDiffNorm * fieldPresetPosDiffScale;
                }
                */
            }
        }
    }

}
