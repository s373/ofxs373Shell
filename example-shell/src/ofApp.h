// ofxs373Shell
// Copyright s373.net/x, 2016
// programmed by Andre Sier 2010,2016

#pragma once

#include "ofMain.h"

#include "ofxs373Shell.h"

#define SR 44100
// #define SR 22050
#define BS 512

class ofApp : public ofBaseApp{

	public:

		ofxs373Shell shell;

		ofSoundStream	sound;

		void setup(){

			ofSetWindowTitle("ofxs373Shell");

			// in this example, 3 concorrent threads access shellinfo
			// opngl thread, audiothread, and ofxs373Shell's thread
			ofSetFrameRate(1000);// or 10000

			string call = "ls -R /home";


			// must have previledges
			string command = "tcpdump -i wlan0 -X";

			// setup was bufferSize maxnumsamples, now is
			// bufferSize numbuffers

			shell.setup(command, 512, 10);//44100*10);
			// shell.setup(call, 512, 4096);
			shell.setup(call, 512, 20);//144096);

			sound.listDevices();
			sound.setDeviceID(3);
			sound.setup(this, 2, 0, SR, BS, 4);
		}


		void keyPressed(int key){
			if(key=='1'){
				shell.setSystemCall("cd  ../ && make"); // not buggy anymore
				// shell.setSystemCall("cd  ../ && make", 512, 144000); // remake this proj
			}
			if(key=='2'){
				shell.setup("ls -Ra ../../../../", 512, 5000);// 14000000);
			}
			if(key=='3'){
				shell.setup("ls -Ra /usr/include/linux", 512, 5000);//144000);
			}
			if(key=='4'){
				shell.setup("ls -Ra /usr/include", 512, 5000);//1440000);
			}
			if(key=='5'){
				shell.setup("top", 512, 5000);//14400000);
			}
			if(key=='6'){
				shell.setup("strace -p 1480", 512, 5000);//14400000);
			}
			// refork self
			if(key=='7'){
				shell.setup("cd  ../ && make && make run", 512, 5000);//1440000);
			}

			if(key=='a'){
				shell.setMaxReadBuffer((int) ofRandom(100));
			}

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
			stats += "shell command: "+ofToString(shell.systemcall)+"\n";
			stats += "shell running: "+ofToString(shell.isProcRunning())+"\n";
			stats += "shell bs maxsamps: "+ofToString(shell.minnumsamples)+" "+ofToString(shell.maxnumsamples)+"\n";
			stats += "shell currentbuffer: "+ofToString(shell.readbufferid)+"\n";
			stats += "shell readbuffers: "+ofToString(shell.numbuffersread)+"\n";
			stats += "shell maxnumbuffers: "+ofToString(shell.maxnumbuffers)+"\n";
			stats += "shell maxnumreadbuffers: "+ofToString(shell.maxreadbufferid)+"\n";
			ofDrawBitmapString(stats, 10, 450);

			// cpying
			string buf1 = shell.readNextBufferStr();
			string buf2 = shell.readNextBufferStr();
			string buf3 = shell.readNextBufferStr();
			// string buf1 = shell.getLine(0);
			// string buf2 = shell.getLine(1);
			// string buf3 = shell.getLine(3);

			ofDrawBitmapString(buf1, 20, 10);
			ofDrawBitmapString(buf2, 20, 110);
			ofDrawBitmapString(buf3, 20, 210);

		}



		void audioOut(float * output, int bufferSize, int nChannels){
			//!cpying
			const string & buf = shell.readNextBufferStr();
			const char * audiochar = &buf[0];

			for (int i = 0; i < bufferSize; i++){
				// float sig = audiochar[i] * 0.00787 * 0.25; // parsing -127 127, ie full char data
				float sig = ofMap(ABS(audiochar[i]),0,127,-1,1) * 0.25; // parsing  0 127, ie ascii data
				 output[i*nChannels    ] = sig ;
				 output[i*nChannels + 1] =  sig ;
			}

		}



};
