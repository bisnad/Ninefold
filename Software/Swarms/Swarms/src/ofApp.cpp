#include "ofApp.h"
#include "dab_flock_parameter.h"
#include "dab_flock_euler_integration_2.h"
#include "dab_flock_agent.h"
#include "dab_flock_swarm.h"
#include "dab_flock_simulation.h"
#include "dab_flock_com.h"
#include "dab_flock_osc_control.h"
#include "dab_space_includes.h"
#include "dab_flock_behavior_includes.h"
#include "dab_flock_visual.h"
#include "dab_flock_com.h"
#include "dab_math_roesseler_field_algorithm.h"
#include "dab_geom_line.h"
#include "ofTrueTypeFont.h"
#include "dab_flock_text_tools.h"
#include "dab_flock_serialize.h"
using namespace dab;
using namespace dab::flock;

//--------------------------------------------------------------
void ofApp::setup()
{
	SerializeTools& serializeTools = SerializeTools::get();

	try
	{
		Simulation& simulation = Simulation::get();
		simulation.setUpdateInterval(10.0);

		simulation.com().createOscControl(7400, "127.0.0.1", 7800);
		simulation.com().createSender("FlockSender_Philippe", "192.168.1.103", 9005, false);
		simulation.com().createSender("FlockSender_Ephraim", "192.168.1.104", 9005, false);
		simulation.com().createSender("FlockSender_Debug", "192.168.1.77", 9005, false);
        simulation.com().createSender("FlockSender_dim_0-2", "192.168.1.101", 10002, false);
        simulation.com().createSender("FlockSender_dim_3-5", "192.168.1.103", 10035, false);
        simulation.com().createSender("FlockSender_dim_6-8", "192.168.1.104", 10068, false);
	
		// setup preset position space
		simulation.space().addSpace(std::shared_ptr<space::Space>(new space::Space("preset_position", new space::KDTreeAlg(9))));

		// create preset swarm
		Swarm* preset_swarm = new Swarm("preset_swarm");
        preset_swarm->addParameter("position", { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 });
        preset_swarm->assignNeighbors("position", "preset_position", false, NULL);
        preset_swarm->addParameter("velocity", { 0.1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 });
        
		preset_swarm->addParameter("position_3d", { 0.0, 0.0, 0.0 });
        preset_swarm->addParameter("position_dim_0-2", { 0.0, 0.0, 0.0  });
        preset_swarm->addParameter("position_dim_3-5", { 0.0, 0.0, 0.0  });
        preset_swarm->addParameter("position_dim_6-8", { 0.0, 0.0, 0.0  });

		preset_swarm->addParameter("velocity_3d", { 0.1, 0.0, 0.0 });
        preset_swarm->addParameter("velocity_dim_0-2", { 0.0, 0.0, 0.0  });
        preset_swarm->addParameter("velocity_dim_3-5", { 0.0, 0.0, 0.0  });
        preset_swarm->addParameter("velocity_dim_6-8", { 0.0, 0.0, 0.0  });
        
		preset_swarm->addBehavior("map_pos_3d", ParameterMapBehavior("position", "position_3d"));
		preset_swarm->set("map_pos_3d_map", {
			1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
			});
        preset_swarm->addBehavior("map_pos_dim_0-2", ParameterMapBehavior("position", "position_dim_0-2"));
        preset_swarm->set("map_pos_dim_0-2_map", {
            1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
        });
        preset_swarm->addBehavior("map_pos_dim_3-5", ParameterMapBehavior("position", "position_dim_3-5"));
        preset_swarm->set("map_pos_dim_3-5_map", {
            0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0
        });
        preset_swarm->addBehavior("map_pos_dim_6-8", ParameterMapBehavior("position", "position_dim_6-8"));
        preset_swarm->set("map_pos_dim_6-8_map", {
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0
        });
        
		preset_swarm->addBehavior("map_vel_3d", ParameterMapBehavior("velocity", "velocity_3d"));
		preset_swarm->set("map_vel_3d_map", {
			1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
			});
        preset_swarm->addBehavior("map_vel_dim_0-2", ParameterMapBehavior("velocity", "velocity_dim_0-2"));
        preset_swarm->set("map_vel_dim_0-2_map", {
            1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
        });
        preset_swarm->addBehavior("map_vel_dim_3-5", ParameterMapBehavior("velocity", "velocity_dim_3-5"));
        preset_swarm->set("map_vel_dim_3-5_map", {
            0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0
        });
        preset_swarm->addBehavior("map_vel_dim_6-8", ParameterMapBehavior("velocity", "velocity_dim_6-8"));
        preset_swarm->set("map_vel_dim_6-8_map", {
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0
        });
        
        /*
        preset_swarm->addBehavior("print_pos", ParameterPrintBehavior("position", ""));
        preset_swarm->addBehavior("print_pos_dim_3-5", ParameterPrintBehavior("position_dim_3-5", ""));
        */
        
		int presetCount = 4;

        preset_swarm->addAgents(presetCount);
        //preset_swarm->randomize("position", { -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0  }, { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 });
		preset_swarm->set(0, "position", { -2.0, -2.0, -2.0, -2.0, -2.0, -2.0, -2.0, -2.0, -2.0 });
		preset_swarm->set(1, "position", { -2.0, 2.0, -2.0, 2.0, -2.0, 2.0, -2.0, 2.0, -2.0 });
		preset_swarm->set(2, "position", { 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0 });
		preset_swarm->set(3, "position", { 2.0, -2.0, 2.0, -2.0, 2.0, -2.0, 2.0, -2.0, 2.0 });

		// change preset visibility
		int visiblePresetIndex = 0;
		std::vector<dab::flock::Agent*>& preset_agents = preset_swarm->agents();
		for (int agentI = 0; agentI < presetCount; ++agentI)
		{
			if(agentI != visiblePresetIndex) preset_agents[agentI]->assignNeighbors("position", "preset_position", false);
		}

		// create regular flocking agent space
		simulation.space().addSpace(std::shared_ptr<space::Space>(new space::Space("agent_position", new space::KDTreeAlg(9))));

		Swarm* swarm = new Swarm("swarm");
		swarm->addParameter("position", { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 });
		swarm->assignNeighbors("position", "agent_position", true, new space::NeighborGroupAlg(3.0, 8, true));
		swarm->assignNeighbors("position", "preset_position", false, new space::NeighborGroupAlg(60.0, 4, true));
		swarm->addParameter("velocity", { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0  });
		swarm->addParameter("acceleration", { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0  });
		swarm->addParameter("force", { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0  });
		swarm->addParameter("mass", { 0.1f });
        
		swarm->addParameter("position_3d", { 0.0, 0.0, 0.0 });
        swarm->addParameter("position_dim_0-2", { 0.0, 0.0, 0.0  });
        swarm->addParameter("position_dim_3-5", { 0.0, 0.0, 0.0  });
        swarm->addParameter("position_dim_6-8", { 0.0, 0.0, 0.0  });

		swarm->addParameter("velocity_3d", { 0.0, 0.0, 0.0 });
        swarm->addParameter("velocity_dim_0-2", { 0.0, 0.0, 0.0  });
        swarm->addParameter("velocity_dim_3-5", { 0.0, 0.0, 0.0  });
        swarm->addParameter("velocity_dim_6-8", { 0.0, 0.0, 0.0  });
        
		swarm->addBehavior("map_pos_3d", ParameterMapBehavior("position", "position_3d"));
		swarm->set("map_pos_3d_map", {
			1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
			});
        swarm->addBehavior("map_pos_dim_0-2", ParameterMapBehavior("position", "position_dim_0-2"));
        swarm->set("map_pos_dim_0-2_map", {
            1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
        });
        swarm->addBehavior("map_pos_dim_3-5", ParameterMapBehavior("position", "position_dim_3-5"));
        swarm->set("map_pos_dim_3-5_map", {
            0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0
        });
        swarm->addBehavior("map_pos_dim_6-8", ParameterMapBehavior("position", "position_dim_6-8"));
        swarm->set("map_pos_dim_6-8_map", {
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0
        });
        
		swarm->addBehavior("map_vel_3d", ParameterMapBehavior("velocity", "velocity_3d"));
		swarm->set("map_vel_3d_map", {
			1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
			});
        swarm->addBehavior("map_vel_dim_0-2", ParameterMapBehavior("velocity", "velocity_dim_0-2"));
        swarm->set("map_vel_dim_0-2_map", {
            1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
        });
        swarm->addBehavior("map_vel_dim_3-5", ParameterMapBehavior("velocity", "velocity_dim_3-5"));
        swarm->set("map_vel_dim_3-5_map", {
            0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0
        });
        swarm->addBehavior("map_vel_dim_6-8", ParameterMapBehavior("velocity", "velocity_dim_6-8"));
        swarm->set("map_vel_dim_6-8_map", {
            1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
        });
        
		swarm->addBehavior("resetForce", ResetBehavior("", "force"));

		swarm->addBehavior("randomize", RandomizeBehavior("", "force"));
		swarm->set("randomize_range", { 0.0001f, 0.0001f, 0.0001f, 0.0001f, 0.0001f, 0.0001f, 0.0001f, 0.0001f, 0.0001f });

		swarm->addBehavior("damping", DampingBehavior("velocity", "force"));
		swarm->set("damping_prefVelocity", { 0.5 });
		swarm->set("damping_amount", { 0.1f });

		swarm->addBehavior("targetPos", TargetParameterBehavior("position", "force"));
		swarm->set("targetPos_target", { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 });
		swarm->set("targetPos_adapt", { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 });
		swarm->set("targetPos_amount", 0.0); // 0.52 // 0.0
		swarm->set("targetPos_absolute", 0.0);

		swarm->addBehavior("targetVel", TargetParameterBehavior("velocity", "force"));
		swarm->set("targetVel_target", { 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 });
		swarm->set("targetVel_adapt", { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 });
		swarm->set("targetVel_amount", 0.0); // 0.52 // 0.0
		swarm->set("targetVel_absolute", 0.0);

		swarm->addBehavior("cohesion", CohesionBehavior("position@agent_position", "force"));
		swarm->set("cohesion_minDist", { 0.0 });
		swarm->set("cohesion_maxDist", { 3.0 });
		swarm->set("cohesion_amount", { 0.0f });

		swarm->addBehavior("alignment", AlignmentBehavior("position@agent_position:velocity", "force"));
		swarm->set("alignment_minDist", { 0.0 });
		swarm->set("alignment_maxDist", { 3.0 });
		swarm->set("alignment_amount", { 0.0f });

		swarm->addBehavior("evasion", EvasionBehavior("position@agent_position", "force"));
		swarm->set("evasion_maxDist", { 0.2f });
		swarm->set("evasion_amount", { 0.0f });

		swarm->addBehavior("preset_cohesion", CohesionBehavior("position@preset_position", "force"));
		swarm->set("preset_cohesion_minDist", { 0.0 });
		swarm->set("preset_cohesion_maxDist", { 60.0 });
		swarm->set("preset_cohesion_amount", { 0.1f });

		swarm->addBehavior("preset_alignment", AlignmentBehavior("position@preset_position:velocity", "force"));
		swarm->set("preset_alignment_minDist", { 0.0 });
		swarm->set("preset_alignment_maxDist", { 3.0 });
		swarm->set("preset_alignment_amount", { 0.0f });

		swarm->addBehavior("preset_evasion", EvasionBehavior("position@preset_position", "force"));
		swarm->set("preset_evasion_maxDist", { 0.2f });
		swarm->set("preset_evasion_amount", { 0.0f });

		swarm->addBehavior("boundaryWrap", BoundaryWrapBehavior("position", "position"));
		swarm->set("boundaryWrap_lowerBoundary", { -5.0, -5.0, -5.0, -5.0, -5.0, -5.0, -5.0, -5.0, -5.0 });
		swarm->set("boundaryWrap_upperBoundary", { 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0 });

		swarm->addBehavior( "boundaryRepulsion", BoundaryRepulsionBehavior( "position", "force" ) );
		swarm->set( "boundaryRepulsion_lowerBoundary", { -5.0, -5.0, -5.0, -5.0, -5.0, -5.0, -5.0, -5.0, -5.0 } );
		swarm->set( "boundaryRepulsion_upperBoundary", { 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0 } );
		swarm->set( "boundaryRepulsion_maxDist", { 2.5 } );
		swarm->set( "boundaryRepulsion_amount", { 0.13 } );

		swarm->addBehavior("boundaryMirror", BoundaryMirrorBehavior("position velocity force", "velocity force"));
		swarm->set("boundaryMirror_lowerBoundary", { -5.0, -5.0, -5.0, -5.0, -5.0, -5.0, -5.0, -5.0, -5.0 });
		swarm->set("boundaryMirror_upperBoundary", { 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0 });
		swarm->set("boundaryMirror_active", 0.0);

		swarm->addBehavior("acceleration", AccelerationBehavior("mass velocity force", "acceleration"));
		swarm->set("acceleration_maxAngularAcceleration", { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 });

		swarm->addBehavior("integration", EulerIntegration2("position velocity acceleration", "position velocity"));
		swarm->set("integration_timestep", { 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1 });

		//swarm->addBehavior( "print", ParameterPrintBehavior("position", "") );
        //swarm->addBehavior( "print", ParameterPrintBehavior("position_dim_0-2", "") );

		swarm->addAgents(16);
		//swarm->addAgents(12);
		swarm->randomize("position", { -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0 }, { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 });

		simulation.com().registerParameter("FlockSender_Philippe", "swarm", "position", swarm->agentCount());
		simulation.com().registerParameter("FlockSender_Philippe", "swarm", "velocity", swarm->agentCount());
		simulation.com().registerParameter("FlockSender_Ephraim", "swarm", "position", swarm->agentCount());
		simulation.com().registerParameter("FlockSender_Ephraim", "swarm", "velocity", swarm->agentCount());
		simulation.com().registerParameter("FlockSender_Debug", "swarm", "position", swarm->agentCount());
		simulation.com().registerParameter("FlockSender_Debug", "swarm", "velocity", swarm->agentCount());
        
        simulation.com().registerParameter("FlockSender_dim_0-2", "preset_swarm", "position_dim_0-2", swarm->agentCount());
        simulation.com().registerParameter("FlockSender_dim_3-5", "preset_swarm", "position_dim_3-5", swarm->agentCount());
        simulation.com().registerParameter("FlockSender_dim_6-8", "preset_swarm", "position_dim_6-8", swarm->agentCount());
        simulation.com().registerParameter("FlockSender_dim_0-2", "preset_swarm", "velocity_dim_0-2", swarm->agentCount());
        simulation.com().registerParameter("FlockSender_dim_3-5", "preset_swarm", "velocity_dim_3-5", swarm->agentCount());
        simulation.com().registerParameter("FlockSender_dim_6-8", "preset_swarm", "velocity_dim_6-8", swarm->agentCount());

        
        simulation.com().registerParameter("FlockSender_dim_0-2", "swarm", "position_dim_0-2", swarm->agentCount());
        simulation.com().registerParameter("FlockSender_dim_3-5", "swarm", "position_dim_3-5", swarm->agentCount());
        simulation.com().registerParameter("FlockSender_dim_6-8", "swarm", "position_dim_6-8", swarm->agentCount());
        simulation.com().registerParameter("FlockSender_dim_0-2", "swarm", "velocity_dim_0-2", swarm->agentCount());
        simulation.com().registerParameter("FlockSender_dim_3-5", "swarm", "velocity_dim_3-5", swarm->agentCount());
        simulation.com().registerParameter("FlockSender_dim_6-8", "swarm", "velocity_dim_6-8", swarm->agentCount());

		FlockVisuals& visuals = FlockVisuals::get();
		visuals.showSwarm("swarm", "position_3d", "velocity_3d", 1);
		visuals.showSwarm("preset_swarm", "position_3d", "velocity_3d", 1);
        
        visuals.setAgentColor("swarm", {0.0, 0.0, 0.0, 0.5});
        visuals.setTrailColor("swarm", { 0.0, 0.0, 0.0, 0.5 });
        visuals.setAgentColor("preset_swarm", {1.0, 0.0, 0.0, 0.5});
        visuals.setTrailColor("preset_swarm", { 1.0, 0.0, 0.0, 0.5 });

		//visuals.showSwarm("swarm", "position", "");
		visuals.setAgentScale("swarm", 0.05);
		visuals.setAgentScale("preset_swarm", 0.05);

		/*
		visuals.showSpace("agent_position");
		visuals.setSpaceColor("agent_position", std::array<float, 4>({ 0.0, 0.0, 0.0, 0.2 }));
		visuals.showSpace("preset_position");
		visuals.setSpaceColor("preset_position", std::array<float, 4>({ 1.0, 0.0, 0.0, 0.2 }));
		*/

		visuals.setDisplayPosition(ofVec3f(0, 0.0, -80.0));
		visuals.setDisplayZoom(0.1);
		visuals.setDisplayOrientation(ofQuaternion(0.0, 0.0, 0.0, 1.0));

		simulation.start();
		simulation.setUpdateInterval(20.0);
	}
	catch (dab::Exception& e)
	{
		std::cout << e << "\n";
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	mIsFullScreen = false;
	mFullScreenSize = { 960, 600 };
	mFullScreenPos = { 50, 50 };

	// for screen recording

	mFullScreenSize = { 951, 568 };
	mFullScreenPos = { -1920, 0 };

}

//--------------------------------------------------------------
void ofApp::update()
{
	//std::cout << "ofApp::update() begin\n";

	//Simulation::get().update();
	FlockVisuals::get().update();

	//std::cout << "ofApp::update() end\n";
}

//--------------------------------------------------------------
void ofApp::draw()
{
	//std::cout << "ofApp::draw() begin\n";

	//FlockVisuals::get().update();
	FlockVisuals::get().display();

	//std::cout << "ofApp::draw() end\n";
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	if (key == 'f')
	{
		ofGetMainLoop()->getCurrentWindow()->toggleFullscreen();

		if (mIsFullScreen == false)
		{
			ofGetMainLoop()->getCurrentWindow()->setWindowShape(mFullScreenSize[0], mFullScreenSize[1]);
			ofGetMainLoop()->getCurrentWindow()->setWindowPosition(mFullScreenPos[0], mFullScreenPos[1]);
			mIsFullScreen = true;
		}
		else
		{
			ofGetMainLoop()->getCurrentWindow()->setWindowShape(1280, 720);
			ofGetMainLoop()->getCurrentWindow()->setWindowPosition(0, 50);
			mIsFullScreen = false;
		}
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}

