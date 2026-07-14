#pragma once

#include <stdio.h>
#include "dab_osc_receiver.h"
#include "dab_flock_swarm.h"
#include "dab_space_includes.h"

class FlockStateListener : public dab::OscListener
{
public:
    FlockStateListener( dab::flock::Swarm* pPresetFlock, dab::flock::Swarm* pFlock, dab::space::SpaceGrid* pSpaceGrid);
    
    void notify( std::shared_ptr<dab::OscMessage> pMessage );
    
protected:
    dab::flock::Swarm* mPresetFlock;
    dab::flock::Swarm* mFlock;
    dab::space::SpaceGrid* mSpaceGrid;
    
    void setPresetFlockPosition(const std::vector< dab::_OscArg* >& pOscArguments);
    void setPresetFlockVelocity(const std::vector< dab::_OscArg* >& pOscArguments);
    void setFlockPosition(const std::vector< dab::_OscArg* >& pOscArguments);
    void setFlockVelocity(const std::vector< dab::_OscArg* >& pOscArguments);
    
    void updateSpaceGrid();
};
