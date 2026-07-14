#pragma once

#include "ofMain.h"
#include "dab_osc_receiver.h"
#include "dab_flock_swarm.h"
#include "dab_space_includes.h"
#include "dab_com_flock_state_listener.hpp"

class ofApp : public ofBaseApp
{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

	protected:
    
		bool mIsFullScreen;
		std::array<int, 2> mFullScreenSize;
		std::array<int, 2> mFullScreenPos;
        std::shared_ptr<FlockStateListener> mFlockStateListener;
    
};
