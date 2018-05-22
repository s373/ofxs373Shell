// ofxs373Shell
// Copyright s373.net/x, 2016
// programmed by Andre Sier 2010,2016
#ifndef _as_h_included_
#define _as_h_included_
#include "ofxs373Shell.h"
#include "ofMain.h"
#define SR 44100
#define BS 512
class ofApp : public ofBaseApp{
	public:
		ofxs373Shell shell;
		ofSoundStream	sound;
		float volume;
		void setup(){
			volume = 0.05;
			ofSetWindowTitle("ofxs373Shell");
			// in this example, 4 concorrent threads access shellinfo
			// opngl thread, audiothread,keyevents, and ofxs373Shell's thread
			ofSetFrameRate(7);// or 10000
			shell.setup("", 512, 20);
		}
		void keyPressed(int key){
		}
		void update(){
		}
		void draw(){
			if(shell.isProcRunning()){
				ofSetColor(0,255,0);
			} else
				ofSetColor(255,0,0);

			ofRect(10,410,50,50);
			ofSetColor(0);
			string stats = "fps: "+ofToString(ofGetFrameRate())+"\n";
			stats += "shell command: "+shell.getSystemCall()+"\n";
			stats += "shell running: "+ofToString(shell.isProcRunning())+"\n";
			stats += "shell bs maxsamps: "+ofToString(shell.getBufferSize())+" "+ofToString(shell.getMaxSamples())+"\n";
			stats += "shell head: "+ofToString(shell.getBufferHead())+"\n";
			stats += "shell headread: "+ofToString(shell.getBufferReadHead())+"\n";
			stats += "shell totalsamples: "+ofToString(shell.getTotalSamples())+"\n";
			ofDrawBitmapString(stats, 10, 450);

			const string buf1 = shell.read();
			const string buf2 = shell.read(false);
			const string buf3 = shell.read();
			ofDrawBitmapString(buf1, 20, 10);
			ofDrawBitmapString(buf2, 20, 110);
			ofDrawBitmapString(buf3, 20, 210);
		}
};
#endif
