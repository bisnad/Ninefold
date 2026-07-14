#include "ofApp.h"
#include "dab_flock_parameter.h"
#include "dab_flock_euler_integration.h"
#include "dab_flock_agent.h"
#include "dab_flock_swarm.h"
#include "dab_flock_simulation.h"
#include "dab_flock_com.h"
#include "dab_osc_receiver.h"
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
#include <Eigen/Dense>

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

		//simulation.com().createOscControl(11002, "127.0.0.1", 7800);
        //simulation.com().createOscControl(11035, "127.0.0.1", 7800);
        simulation.com().createOscControl(11068, "127.0.0.1", 7800);

		// setup preset position space
		simulation.space().addSpace(std::shared_ptr<space::Space>(new space::Space("preset_position", new space::KDTreeAlg(3))));

		// create preset swarm
		Swarm* preset_swarm = new Swarm("preset_swarm");
        preset_swarm->addParameter("position", { 0.0, 0.0, 0.0 });
		preset_swarm->assignNeighbors("position", "preset_position", false, NULL);
        preset_swarm->addParameter("velocity", { 0.0, 0.0, 0.0 });
        
		int presetCount = 4;

        preset_swarm->addAgents(presetCount);

		// create regular flocking agent space
		simulation.space().addSpace(std::shared_ptr<space::Space>(new space::Space("agent_position", new space::KDTreeAlg(3))));

		Swarm* swarm = new Swarm("swarm");
		swarm->addParameter("position", { 0.0, 0.0, 0.0 });
		swarm->assignNeighbors("position", "agent_position", true, new space::NeighborGroupAlg(3.0, 8, true));
		swarm->assignNeighbors("position", "preset_position", false, new space::NeighborGroupAlg(60.0, 4, true));
		swarm->addParameter("velocity", { 0.0, 0.0, 0.0 });

		swarm->addAgents(16);

        // create grid space
        //space::SpaceGrid* spaceGrid = new space::SpaceGrid(3, Array<unsigned int>{ 20, 20, 20 }, Eigen::Vector3f(-5.0, -5.0, -5.0), Eigen::Vector3f(5.0, 5.0, 5.0));
        space::SpaceGrid* spaceGrid = new space::SpaceGrid(3, Array<unsigned int>{ 20, 20, 20 }, Eigen::Vector3f(-5.0, -5.0, -5.0), Eigen::Vector3f(5.0, 5.0, 5.0));
        simulation.space().addSpace(std::shared_ptr<space::Space>(new space::Space("spacegrid", new space::GridAlg(spaceGrid, space::GridAlg::AvgLocationMode, space::GridAlg::NoUpdateMode))));
        
        //std::shared_ptr<dab::OscReceiver> flockStateReceiver = simulation.com().addReceiver(std::shared_ptr<dab::OscReceiver>( new dab::OscReceiver("FlockStateReceiver", 10002)));
        //std::shared_ptr<dab::OscReceiver> flockStateReceiver = simulation.com().addReceiver(std::shared_ptr<dab::OscReceiver>( new dab::OscReceiver("FlockStateReceiver", 10035)));
        std::shared_ptr<dab::OscReceiver> flockStateReceiver = simulation.com().addReceiver(std::shared_ptr<dab::OscReceiver>( new dab::OscReceiver("FlockStateReceiver", 10068)));
        mFlockStateListener = std::shared_ptr<FlockStateListener>( new FlockStateListener( preset_swarm, swarm, spaceGrid ) );
        flockStateReceiver->registerOscListener(mFlockStateListener);
        

		FlockVisuals& visuals = FlockVisuals::get();
		visuals.setDisplayColor({ 0.0, 0.0, 0.0, 1.0 });

		visuals.showSwarm("swarm", "position", "velocity", 10000);
        visuals.setAgentColor("swarm", {1.0, 1.0, 1.0, 0.5});
		visuals.setAgentScale("swarm", 0.02);
        visuals.setTrailColor("swarm", { 1.0, 1.0, 1.0, 0.5 });

		visuals.showSwarm("preset_swarm", "position", "", 10);
		visuals.setAgentColor("preset_swarm", {1.0, 1.0, 1.0, 0.5});
		visuals.setAgentScale("preset_swarm", 0.02);
		visuals.setTrailColor("preset_swarm", { 1.0, 1.0, 1.0, 0.5 });

		visuals.showSpace("agent_position");
		visuals.setSpaceColor("agent_position", std::array<float, 4>({ 1.0, 0.0, 0.0, 0.2 }));
		
		visuals.showSpace("preset_position");
		visuals.setSpaceColor("preset_position", std::array<float, 4>({ 0.0, 1.0, 0.0, 1.0 }));

        visuals.showSpace("spacegrid");
        visuals.setSpaceColor("spacegrid", std::array<float,4>( { 1.0, 1.0, 1.0, 0.2 } ));
		visuals.setSpaceValueScale("spacegrid", 0.4);

        visuals.setDisplayPosition(ofVec3f(0, 0.0, -80.0));
        visuals.setDisplayZoom(0.1);
        visuals.setDisplayOrientation(ofQuaternion(0.0, 0.0, 0.0, 1.0));

		simulation.start();
		simulation.setUpdateInterval(20.0);

		//simulation.com().receiver("ControlReceiver")->setListenInterval(50);
		//flockStateReceiver->setListenInterval(50);


	}
	catch (dab::Exception& e)
	{
		std::cout << e << "\n";
	}

	//ofSetBackgroundAuto(false);
	//glDisable(GL_DEPTH_TEST);
	//ofSetFrameRate(60);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	mIsFullScreen = true;
	mFullScreenSize = {1920, 1200}; 
	mFullScreenPos = { 0, 0 }; // Display Dims 6 - 8
	ofHideCursor();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	ofSetFrameRate(30);
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
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glDisable(GL_DEPTH_TEST);

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
			ofHideCursor();
			mIsFullScreen = true;
		}
		else
		{ 
			ofGetMainLoop()->getCurrentWindow()->setWindowShape(1280, 720);
			ofGetMainLoop()->getCurrentWindow()->setWindowPosition(0, 50);
			ofShowCursor();
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

