// ofxs373Shell
// Copyright s373.net/x, 2016
// programmed by Andre Sier 2010,2016

// exec's and captures processes results output
// no parsing, but should be easy

// modeled after inter threading access,
// maintains a safe pool of data around
// initially programmed in 2010 processing s373.shell


// todo: *** Error in `./example-shell': double free or corruption (out): 0x0000560490418510 ***Aborted (core dumped)

// todo: verify newlines are still not good while parsing proc, they are good once we cpy the whole string

#pragma once

#include <iostream>
#include <stdio.h>
#include "Poco/Mutex.h"
#include "ofMain.h"

#define SHELLMAXSTRING 1<<21

class ofxs373Shell : public ofThread{

public:

   	FILE 			*inproc;

	string			systemcall; // command

	string			systemcallresult; // buffered n min samples results

	string 			procmessagebuffer; // internal proc dump
	string 			messagebuffer; // access this from outside, raw incoming data minnumsamples dim

	string 			fullmessagebuffer; // storage

	int 			minnumsamples; //buffer size
	int 			maxnumsamples; //storage, integer multiple of maxnumsamples / minnumsamples
	int 			maxnumbuffers, numbuffersread; //storage, actual number of bufs
	int				minreadbufferid,maxreadbufferid;
	int 			readbufferid, writebufferid; //write still not used

	bool 			hasinfo;
	bool 			procrunning;
	bool 			copywhendone,verbose;

	/// lines int processing
	int numlines;
	Poco::FastMutex mutex;

	~ofxs373Shell(){}
	ofxs373Shell(){
		procrunning=false;
		minreadbufferid=maxreadbufferid = -1;
	}

	// recalling setup was working ok gcc5, now
	void setup(string scall, int minsamples=64,
		// int maxsamples=10000,
		int maxbuffers=1000,
		bool cpy=false, bool verb=false){


		if(procrunning){
			if(inproc!=NULL){ pclose(inproc);}
		}
		inproc=NULL;
		readbufferid = writebufferid = numlines = 0;
		hasinfo = false;
		procrunning = false;
		copywhendone = cpy;
		verbose = verb;
		systemcall = scall;
		systemcallresult = "";
		setSamples(minsamples,maxbuffers);
		setSystemCall(scall);
	}

	// thread unsafe, be sure to init this first b4 acessing
	void setSamples(int min, int nbuf){

		mutex.lock();

		minnumsamples=min; //bs
		// int desiredmaxnumsamples=min*nbuf; //max;

		// float numbuffers = (float)desiredmaxnumsamples/minnumsamples;

		// maxnumbuffers = (int) (numbuffers+1);
		maxnumbuffers = nbuf;

		maxnumsamples = minnumsamples * maxnumbuffers;


		numbuffersread = 0;

		if(verbose)
		cout << this <<" ofxs373Shell minsamples " << minnumsamples<<
		" numbfs " << maxnumbuffers <<
		 " actualsamples " << maxnumsamples << endl;

		// gather memory before start, epic amounts of zeros
		 procmessagebuffer="";
		 messagebuffer="";
		 // messagebufferret="";
		 fullmessagebuffer="";

		for(int i=0; i<maxnumsamples; i++){
			fullmessagebuffer += '\0';
		}
		for(int i=0; i<minnumsamples; i++){
			messagebuffer+=fullmessagebuffer[i];
		}
		procmessagebuffer = messagebuffer;

		mutex.unlock();
	}

	void setSystemCall(string call){

		if(isThreadRunning()){
			stopThread();
			if(inproc){
				pclose(inproc);
				inproc=NULL;
			}
		}
		systemcall = call;
		numbuffersread = 0;

		if(isThreadRunning())stopThread();
		if(!isThreadRunning()) startThread();
	}


	bool isProcRunning(){
		return procrunning;
	}

	void setMinReadBuffer(int i){ minreadbufferid = i; }
	void setMaxReadBuffer(int i){ maxreadbufferid = i; }

   void threadedFunction(){

   		while (isThreadRunning()) {

reinit:
 			ofSleepMillis(5);

			if(systemcall.size()<=1){
					cout << this << " systemcall empty " << endl;
   		    		stopThread(); return;
			}

   			// FILE *inproc;
   			char * buf = &procmessagebuffer[0];

			// read priv
   		   	if( !(inproc=popen(systemcall.c_str(), "r" )) ) {
   		   		cout << "no inproc error , stopThread()" << endl;
   		   		stopThread(); return;
   	    	}

   	    	hasinfo = false;
   	    	procrunning = false;
   	    	numbuffersread = 0;
			systemcallresult = "";
			// string bufferresult="";

			int numiters = -1;
			int numsamplesread = 0;
			numlines = 0;

			if(verbose) cout << "ofxs373Shell begin thread w command " << systemcall << endl;

   		    while( fgets(buf,minnumsamples,inproc) != NULL){
   	    			procrunning = true;

					numiters++;

					bool ended = false;

					int howmanythisloop=0;
					int oldstringstartpos = systemcallresult.size();

					for(int i=0; i<minnumsamples; i++){
						char c = procmessagebuffer[i];
						if(c=='\0'||c==10||c==13){ // fegets is null terminated c sstr in a sintring buffer
							ended = true;
							// systemcallresult << endl;
							systemcallresult+='\n';
							// systemcallresult+='\n'; // n no systemcall mas no buffer
							numlines++;
							break; // fgetsbreaks on newlines & nullch
						}

						systemcallresult += c;
						howmanythisloop++;
					}


					if(systemcallresult.size()>SHELLMAXSTRING){
						// hardreset
						// systemcallresult = "";

						pclose(inproc);
						goto reinit;
					}


							// systemcallresult+='\n';


					numsamplesread += howmanythisloop;

					// from read samples, determine bufferid and startindexwriting

					int bufferidstart = numsamplesread / minnumsamples;

					if(numbuffersread<=bufferidstart){
						numbuffersread=bufferidstart;
					}


					int sampleidstart = numsamplesread % minnumsamples;

					int startidx = oldstringstartpos;
					int startmessageidx = oldstringstartpos % minnumsamples;

					if(ended){
						howmanythisloop++;
					}


					if(mutex.tryLock(5)) {

						for(int i=0; i<howmanythisloop; i++){

							char c = 0;

							if(ended && i==(howmanythisloop-1)){
								c = '\n';
							} else {
								c = systemcallresult[startidx+i];
							}


							// int midx = (startmessageidx + i) % minnumsamples;
							// messagebuffer[midx] = c;

							int bidx = bufferidstart * minnumsamples + sampleidstart + i;
							while(bidx>=(maxnumsamples-1)){ bidx -= maxnumsamples; }
							fullmessagebuffer[bidx] = c;
						}

						mutex.unlock();
					}




			} // end proc

 			int status = pclose(inproc);
 			// numlines = countNumLines(); // reset numlines

			if(verbose)
 			cout << this <<" ofxs373Shell closed proc w status " << status <<
 			" reading "<< systemcallresult.size() << " chars " << endl
 			<< " numiters " << numiters << " bufsize " << minnumsamples << " numbuffersread " << numbuffersread
 			<< " maxnumsamples " << maxnumsamples << " numlines "<< numlines<<endl;


 			procrunning = false;


 			ofSleepMillis(5);
 			//cout << "FULL MESSAGE size "<< systemcallresult.size() << endl;
 			// cout << systemcallresult << endl;

 			// n devia ser preciso, acima deve ter erros,
 			// passar tudo da mesnage dentro do buffer

 			if(copywhendone){

	 			numlines = countNumLines(); // reset numlines


	 			int msize = systemcallresult.size();
	 			int msamp = maxnumsamples;

	 			if(msize>msamp){
	 				cout << "warning proc message size > proc message buffer " << msize << " " << msamp << endl;
	 			}

	 			if(verbose) cout << "begin fullproc message buffer " << msize << " " << msamp << endl;

				if(mutex.tryLock(5)) {

		 			for(int i=0; i<msamp; i++ ){
		 				if( i < msize){
		 					fullmessagebuffer[i] = systemcallresult[i];
		 				}else {
		 					fullmessagebuffer[i] = 32;
		 				}
		 			}

					mutex.unlock();
				}
 			}

 			stopThread();



 			// ofSleepMillis(50);
		} // while thread running

	} // func



	// reading samples

	const string & readBufferStrSampleId(int startsampleid){
		int startid = startsampleid;
		int safeid = numbuffersread * minnumsamples - minnumsamples -1;
		if(startid >= safeid){
			startid = safeid;
			cout << "adjusting readBufferStr startid to " << startid << endl;
		}

		mutex.lock();
		for(int i=0; i<minnumsamples; i++){
			messagebuffer[i] = fullmessagebuffer[startid+i];
		}
		mutex.unlock();

		return messagebuffer;
	}

	const string & readBufferStrBufferId(int startbufferid){
		// int maxbufs = MIN(maxnumbuffers, numbuffersread);
		int safebufferid = MIN(startbufferid, maxnumbuffers);
		int startid = safebufferid * minnumsamples;
		mutex.lock();
		for(int i=0; i<minnumsamples; i++){
			messagebuffer[i] = fullmessagebuffer[startid+i];
		}
		mutex.unlock();
		return messagebuffer;
	}
	const string & readNextBufferStr(){

		int maxbufs = numbuffersread < maxnumbuffers ? numbuffersread : maxnumbuffers; //MIN(maxnumbuffers, numbuffersread);
		if(maxbufs<=0){
			// account no info yet
			return messagebuffer;
		}

		if(maxreadbufferid>0){
			maxbufs = MIN(maxbufs, maxreadbufferid);
		}

		// readbufferid = (readbufferid+1)%maxbufs; // % is not fast
		readbufferid++;
		if(readbufferid>=(maxbufs-1)) {
			if(minreadbufferid < 0) readbufferid=0;
			else readbufferid = minreadbufferid;
		}
		return readBufferStrBufferId(readbufferid);
	}

	// allocates & copies, not fast but handy
	const string readStr(int startidx, int numsamptstoread){
		string ret="";
		mutex.lock();
		for(int i=0; i<numsamptstoread; i++){
			int idx = (startidx+i)%maxnumsamples;
			ret += fullmessagebuffer[idx];
		}
		mutex.unlock();
		return ret;
	}

	const string getLine(int n){

		string linestr="";

		if(numlines<=0){
			cout << this << " s373ShellThread warning no linesyet " << n << " w numlines " << numlines << endl;
			return linestr;
		}

		int clineidx =0;
		if(n>=numlines){
			cout << this << " s373ShellThread warning getLine " << n << " w numlines " << numlines << endl;
			n = numlines -1;
		}

		mutex.lock();
		for(int i=0; i<maxnumsamples;i++){
			if(clineidx == n){ linestr += fullmessagebuffer[i]; }
			if(clineidx > n){ break; }
			if(fullmessagebuffer[i]==10||fullmessagebuffer[i]==13){
				clineidx++;
			}
		}
		mutex.unlock();
		return linestr;
	}

	int countNumLines(){
		int nlines=0;

		mutex.lock();
		for(int i=0; i<maxnumsamples; i++){
			// if(fullmessagebuffer[i]=='\n'){
			if(fullmessagebuffer[i]==10||fullmessagebuffer[i]==13){
				nlines++;
			}
		}
		mutex.unlock();
		return nlines;
	}

};
