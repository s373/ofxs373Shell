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
				shell.exe(julian);
			}
			if(key=='1'){
				shell.exe("cd  ../ && make");
			}
			if(key=='2'){
				shell.exe("ls -Ra ../../");
			}
			if(key=='3'){
				shell.exe("ls -Ra /usr/include/linux");
			}
			if(key=='4'){
				shell.exe("ls -Ra /usr/include");
			}
			if(key=='5'){
				shell.exe("top");
			}
			if(key=='6'){
				shell.exe("strace -p 1480");
			}
			// refork self always
			if(key=='7'){
				shell.exe("cd  ../ && make && make run");
			}

			if(key=='a'){
				int a = (int) ofRandom(shell.getTotalSamples()/5);
				int b =(int) ofRandom(shell.getTotalSamples());
				shell.setMinMax(b,a+b);
			}

		}


		void update(){

			// if(ofGetMousePressed(0)){
            //
			// 	float perx = ofGetMouseX()/(float)ofGetWidth();
			// 	float pery = ofGetMouseY()/(float)ofGetHeight();
			// 	int mn = (int) (perx * shell.getTotalSamples());
			// 	int mx = (int) (pery * shell.getTotalSamples());
			// 	shell.setMinMax(mn,mx);
			// }
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

			// cpying
			const string buf1 = shell.read();
			const string buf2 = shell.read(false);
			const string buf3 = shell.read();

			ofDrawBitmapString(buf1, 20, 10);
			ofDrawBitmapString(buf2, 20, 110);
			ofDrawBitmapString(buf3, 20, 210);

		}



		void audioOut(float * output, int bufferSize, int nChannels){
			//cpying
			const string buf = shell.read();
			const char * audiochar = &buf[0];

			for (int i = 0; i < bufferSize; i++){
				// float sig = audiochar[i] * 0.00787 * 0.25; // parsing -127 127, ie full char data
				float sig = ofMap(ABS(audiochar[i]),0,127,-1,1) * 0.25; // parsing  0 127, ie ascii data regions sort of
				 output[i*nChannels    ] = sig ;
				 output[i*nChannels + 1] =  sig ;
			}

		}



};
