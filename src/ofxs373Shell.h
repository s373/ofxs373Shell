// ofxs373Shell
// Copyright s373.net/x, 2016
// programmed by Andre Sier 2010,2016
#pragma once
#include <iostream>
#include <stdio.h>
#include "Poco/Mutex.h"
#include <string>
#define SHELLMAXSTRING (1<<21)
class ofxs373Shell : public ofThread{
	protected:
		std::string			systemcall;
		std::string			systemresult;
		std::string			bufferstring;
		std::string			readbufferstring;
		std::string			fullbufferstring;
		std::string			fullbufferstringcopy;
		int 				buffersize, bufferhead;
		int 				bufferreadhead, bufferreadmin, bufferreadmax;
		int 				numbuffers;
		int 				maxsamples,totalsamples,totalsamples1;
		Poco::FastMutex		mutex;
		bool 				procrunning;
		bool 				proccancel;
		bool 				copywhendone;
		bool 				updatebufferreadmax;
		bool 				verbose;
	public:
	~ofxs373Shell(){}
	ofxs373Shell(){
		systemcall = "";
		systemresult = "";
		bufferstring = "";
		readbufferstring = "";
		fullbufferstring = "";
		fullbufferstringcopy = "";
		buffersize = numbuffers = totalsamples = bufferreadmax =
		maxsamples = bufferhead = bufferreadmin = totalsamples1 = bufferreadhead = 0;
		procrunning=proccancel=copywhendone=verbose=false;
		updatebufferreadmax=true;
	}
	void setup(string scall, int bs=64,int nbufs=1000,
		bool cpy=false, bool verb=false){
			procrunning = false;
			buffersize = bs;
			numbuffers = nbufs;
			maxsamples = buffersize * numbuffers;
			procrunning = false;
			copywhendone = cpy;
			verbose = verb;
			setSamples(buffersize,numbuffers);
			doSystemCall(scall);
	}
	void setSamples(int min, int nbufs){
			buffersize = min;
			numbuffers = nbufs;
			maxsamples = buffersize * numbuffers;
			bufferstring = "";
			for(int i=0; i<buffersize; i++){
				bufferstring += '\0';
			}
			readbufferstring = bufferstring;
			fullbufferstring = "";
			for(int i=0; i<maxsamples; i++){
				fullbufferstring += '\0';
			}
			fullbufferstringcopy = fullbufferstring;
	}
	void setMinMax(int mn, int mx){
		bufferreadmin = CLAMP(mn,0,totalsamples1);
		bufferreadmax = CLAMP(mx,0,totalsamples1);
	}
	void setUpdatebufferreadmax(bool b){
		updatebufferreadmax =b;
	}
	void doSystemCall(const string & call){
		if(isThreadRunning()){
			// cout << "threadrunning, stopping"  << endl;
			stopThread();
		}
		systemcall = call;
		bufferhead = 0;
		totalsamples = 0;
		bufferreadhead = 0;
		// if(verbose) cout << "starting thread "  << endl;
		if(isThreadRunning())stopThread();
		if(!isThreadRunning()) startThread();
	}
	bool isProcRunning(){
		return procrunning;
	}
   void threadedFunction(){
   		while (isThreadRunning()) {
			FILE 				*proc = NULL;
reinit:
			if(proccancel){
				// if(verbose) cout << "proccancel"  << endl;
				if(proc){
					pclose(proc);
					proc=NULL;
					// if(verbose) cout << "proc closed and null"  << endl;
				}
				proccancel=false;
			}
 			ofSleepMillis(5);
			if(systemcall.size()<=1){
					// cout << this << " systemcall empty " << endl;
   		    		stopThread(); return;
			}
   			char * buf = &bufferstring[0];
			procrunning = false;
			proc = popen(systemcall.c_str(), "r" );
			if(proc==NULL){
				// cout << "no inproc error , stopThread()" << endl;
				stopThread(); return;
			}
			procrunning = true;
			systemresult = "";
			int numiters = -1;
			int numsamplesread = 0;
			settotalsamples(0);
			// if(verbose) cout << "ofxs373Shell begin thread w command " << systemcall << endl;
   		    while( fgets(buf,buffersize,proc) != NULL){
   	    			procrunning = true;
					numiters++;
					bool ended = false;
					int howmanythisloop=0;
					for(int i=0; i<buffersize; i++){
						char c = bufferstring[i];
						if(c=='\0'){
							ended = true;
							break;
						}
						systemresult += c;
						howmanythisloop++;
					}
					if(systemresult.size()>SHELLMAXSTRING){						
						//if(verbose) cout << this << " hardreset!" << endl;
						systemresult = "";
					}
					numsamplesread += howmanythisloop;
					if( numsamplesread < maxsamples )
						settotalsamples(numsamplesread);
					int lastbufferhead = bufferhead;
					if(mutex.tryLock(20)){
						for(int i=0; i<howmanythisloop; i++){
							fullbufferstring[bufferhead] = bufferstring[i];
							bufferhead++;
							if(bufferhead==(maxsamples-1)) bufferhead=0;
						}
						mutex.unlock();
					}
				}
 				int status = pclose(proc);
				//if(verbose)
	 			//cout << this <<" ofxs373Shell closed proc w status " << status <<
	 			//" reading "<< systemresult.size() << " chars " << endl
	 			//<< " numiters " << numiters << " bufsize " << buffersize << " numbuffersread " << numbuffers << " totalsamples " << totalsamples
	 			//<< " maxnumsamples " << maxsamples << endl;
				// " numlines "<< numlines<<endl;
 				procrunning = false;
 				ofSleepMillis(5);
 				stopThread();
		}
	}
	const string read( bool advance = true){
		std::string result = "";
		if(mutex.tryLock(20)){
			if(fullbufferstring.size() > buffersize){
				for(int i=0; i<buffersize; i++){
					int idx = (bufferreadhead + i);
					if(idx>totalsamples1) idx=0; // pode ser zero
					readbufferstring[i] = fullbufferstring[idx];
				}
			}
			result = readbufferstring;
			mutex.unlock();
			if(advance){
				bufferreadhead += buffersize;
				if(bufferreadhead>bufferreadmax) bufferreadhead = bufferreadmin;
			}
		}
		return result;
	}
	const string readBufferLoc( int bloc ){
		std::string result = "";
		if(mutex.tryLock(20)){
			for(int i=0; i<buffersize; i++){
				int idx = (bloc + i);
				if(idx>totalsamples1) idx=0; // pode ser zero
				readbufferstring[i] = fullbufferstring[idx];
			}
		}
		result = readbufferstring;
		mutex.unlock();
		return result;
	}
	void exe(const string & call){
			bufferreadhead = 0;
			proccancel = true;
			doSystemCall(call);
	}
	const int getMaxSamples(){
		return maxsamples;
	}
	const int getTotalSamples(){
		return totalsamples1;
	}
	const int getBufferSize(){
		return buffersize;
	}
	const int getBufferHead(){
		return bufferhead;
	}
	const int getBufferReadHead(){
		return bufferreadhead;
	}
	const string & getSystemCall(){
		return systemcall;
	}
private:
	void settotalsamples(int s){
		totalsamples=s;
		totalsamples1=s>0?s-1:0;
		if(updatebufferreadmax){
			bufferreadmax=totalsamples1;
		}
	}
};
