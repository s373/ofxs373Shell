#pragma once

#include "ofMain.h"

#include "ofxs373Shell.h"

#define SR 44100
#define BS 512

class ofApp : public ofBaseApp{

	public:

		ofxs373Shell shell;

		ofSoundStream	sound;


		void setup(){
	
			ofSetFrameRate(10);

			// string call = "ls -laR";
			// string call = "top";
			// string call = "tree /home/as";
			// string call = "ls -R /home";
			// string call = "ls -R /home";
			string call = "ls -R /home";
			// string call = "top";
			// 0x21a32c8 ofxs373Shell closed proc w status 0reading 2881250 chars

			string command = "tcpdump -i wlan0 -AvvX";

			// shell.setup(call, 32, 1000);
			// shell.setup(call, 512, 44100*10);
			shell.setup(command, 512, 44100*10);
			// shell.setup(call, 512, 4096);
			// shell.setup(call, 512, 144096);

			sound.listDevices();
			sound.setDeviceID(3);
			// sound.setup(this, 2, 0, 44100, 64, 4);
			sound.setup(this, 2, 0, SR, BS, 4);

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
			stats += "shell running: "+ofToString(shell.isProcRunning())+"\n";
			stats += "shell bs maxsamps: "+ofToString(shell.minnumsamples)+" "+ofToString(shell.maxnumsamples)+"\n";
			stats += "shell currentbuffer: "+ofToString(shell.readbufferid)+"\n";
			ofDrawBitmapString(stats, 10, 500);

			string buf1 = shell.readNextBufferStr();
			string buf2 = shell.readNextBufferStr();
			string buf3 = shell.readNextBufferStr();
			// string buf1 = shell.getLine(0);
			// string buf2 = shell.getLine(1);
			// string buf3 = shell.getLine(3);

			ofDrawBitmapString(buf1, 80, 10);
			ofDrawBitmapString(buf2, 380, 10);
			ofDrawBitmapString(buf3, 580, 10);

		}



		void audioOut(float * output, int bufferSize, int nChannels){

			const string & buf = shell.readNextBufferStr();
			const char * audiochar = &buf[0]; 

			for (int i = 0; i < bufferSize; i++){
				float as = audiochar[i] * 0.00787 * 0.2;
				 output[i*nChannels    ] = as ;//+ ofMap(output[0], viz.channels[0].min, viz.channels[0].max, -1, 1) * volume;
				 output[i*nChannels + 1] =  as ;//+ ofMap(output[1], viz.channels[1].min, viz.channels[1].max, -1, 1) * volume;
			}

		}



};
