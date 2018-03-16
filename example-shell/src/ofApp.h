// ofxs373Shell
// Copyright s373.net/x, 2016
// programmed by Andre Sier 2010,2016

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

			ofSetWindowTitle("ofxs373Shell");

			// in this example, 3 concorrent threads access shellinfo
			// opngl thread, audiothread, and ofxs373Shell's thread
			ofSetFrameRate(30);// or 10000

			string call = "ls -R /home";

			// must have previledges
			string command = "tcpdump -i wlan0 -X";


			// setup was syscall bufferSize maxnumsamples, now is
			// setup syscall bufferSize numbuffers

			shell.setup(command, 512, 10);
			shell.setup(call, 512, 20);

			sound.listDevices();
			sound.setDeviceID(6);
		 	sound.setup(this, 2, 0, SR, BS, 4);
		}

		void keyPressed(int key){

			if(key=='0'){
				//‏fun from @julian0liver: https://twitter.com/julian0liver/status/698577959859392512
				string julian = "make V=1 2>&1|while read line;do echo \"$line\"|wc -m|beep -f `tee` -r 3 -l 100; echo $line; done #;-)";
				shell.setSystemCall(julian);
			}
			if(key=='1'){
				shell.setSystemCall("cd  ../ && make");
			}
			if(key=='2'){
				shell.setSystemCall("ls -Ra ../../");
			}
			if(key=='3'){
				shell.setSystemCall("ls -Ra /usr/include/linux");
			}
			if(key=='4'){
				shell.setSystemCall("ls -Ra /usr/include");
			}
			if(key=='5'){
				shell.setSystemCall("top");
			}
			if(key=='6'){
				shell.setSystemCall("strace -p 1480");
			}
			// refork self always
			if(key=='7'){
				shell.setSystemCall("cd  ../ && make && make run");
			}

			if(key=='a'){
				int a = (int) ofRandom(100);
				int b =(int) ofRandom(shell.maxnumbuffers);
				shell.setMinReadBuffer(b);
				shell.setMaxReadBuffer(b+a);
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
				float sig = ofMap(ABS(audiochar[i]),0,127,-1,1) * 0.25; // parsing  0 127, ie ascii data regions sort of
				 output[i*nChannels    ] = sig ;
				 output[i*nChannels + 1] =  sig ;
			}

		}



};
