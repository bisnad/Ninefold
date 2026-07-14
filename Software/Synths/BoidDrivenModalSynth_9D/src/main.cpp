#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){

	//Use ofGLFWWindowSettings for more options like multi-monitor fullscreen
	ofGLFWWindowSettings settings;
	settings.setSize(1, 1);
	settings.setPosition(glm::vec2(1920, 1200));
	settings.windowMode = OF_WINDOW; //can also be OF_FULLSCREEN
	settings.decorated = false;

	auto window = ofCreateWindow(settings);

	ofRunApp(window, std::make_shared<ofApp>());
	ofRunMainLoop();

}
