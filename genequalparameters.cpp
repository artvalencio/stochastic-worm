/* GENEQUALPARAMETERS Generates parameters files for Galves-Locherbach neuron dynamics, assuming all neurons are equal.

INPUT FILES:
- Adjacency matrix: adjacency.csv
- Compressed external stimulii: compressedstimulus.csv
(Compressed stimulii consists of a table: neuron-ID|start-time|duration|intensity)

OUTPUT FILES:
- Neuron input parameters (VR,VB,mu): neuronmodel.csv
- Initial conditions: initial.csv
- External stimulii: stimulus.csv

(C)Arthur Valencio 23 July 2018
Update: 26 July 2018
*/

#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

double randomgen(void);

int main(void){
	//Opening input/output files
	ifstream Adjacency;
	ifstream Cstimulus;
	ofstream Neuronmodel;
	ofstream Initial;
	ofstream Stimulus;
	Adjacency.open("adjacency.csv");
	Cstimulus.open("compressedstimulus.csv");
	Neuronmodel.open("neuronmodel.csv");
	Stimulus.open("stimulus.csv");
	if(Adjacency.fail()||Cstimulus.fail()){
		cout<<"Couldn't open input file adjacency.csv or compressedstimulus.csv"<< endl;
		exit(1);
	}
	//Getting the parameters from user
	int i,j,tslength,transient,nonodes=1;
	string line, strval, line2, line3, line4, line5, strval2, strval3, strval4;
	char choiceinicond;
	double VR,VB,mu;
	cout<<"Inform the time-series length: ";	
	cin>>tslength;
	cout<<"Inform the transient: ";
	cin>>transient;
	cout<<"Neuron model:"<<endl;
	while(1){	
		cout<<"VR=";
		cin>>VR;
		cout<<endl;
		cout<<"VB=";
		cin>>VB;
		cout<<endl;
		if(VR>VB){
			cout<<"VR must be smaller or equal to VB"<<endl;
		}
		else{
			break;
		}
	}
	cout<<"\u03BC=";
	cin>>mu;
	cout<<endl;
	cout<<"Initial conditions. Select: \n <r> for random, \n <n> for all neurons start with V0=VB and X0='0' \n <i> to select starting conditions individually"<<endl;
	cin>>choiceinicond;
	//Get number of neurons
	getline(Adjacency,line);
	for(unsigned int l=0;l<line.size();l++){
		if(line[l]==','){
			nonodes=nonodes+1;
		}
	}
	//Get number of items in Cstimulus
	int count=0;	
	while(getline(Cstimulus,line)){
		count++;
	}
	Cstimulus.close();
	Cstimulus.open("compressedstimulus.csv");
	double V0[nonodes],X0[nonodes];
	//get or generate initial conditions
	if(choiceinicond=='r'){
		double percfiring;
		cout<<"Inform the percentage of firing neurons at initial condition:";
		cin>>percfiring;
		cout<<endl;
		for(i=0;i<nonodes;i++){
			V0[i]=randomgen()*percfiring*VR;
			if(V0[i]<VR){
				V0[i]=VR;
				X0[i]=1;
			}
			else if(V0[i]==VR){
				X0[i]=1;
			}
			else{
				X0[i]=0;
			}	
		}
		
	}
	else if(choiceinicond=='n'){
		for(i=0;i<nonodes;i++){
			V0[i]=VB;
			X0[i]=0;
		}
	}
	else{
		char cont;
		int node;
		cout<<"Please inform the initial potentials V0. \n If V0>VR then X0='0' , otherwise X0='1' and V0=VR. \n All neurons not informed will be set to V0=VB and X0=0"<<endl;
		for(i=0;i<nonodes;i++){
			V0[i]=VB;
			X0[i]=0;
		}
		while(1){
			cout<<"Which neuron (Neuron ID):";
			cin>>node;
			cout<<endl;
			cout<<"Initial potential (V0):";
			cin>>V0[node];
			cout<<endl;
			if(V0[node]>VR){
				X0[node]=0;
			}
			else{
				X0[node]=1;
				V0[node]=VR;
			}
			cout<<"Continue?(y/n)";	
			cin>>cont;		
			if(cont=='y'||cont=='Y'){
				cout<<endl;
				break;
			}
		}
	}
	int neuron[count]={0},timestart[count]={0},duration[count]={0};
	double intensity[count]={0},I[nonodes][tslength+transient]={0};
	for(i=0;i<=count;i++){
        	getline(Cstimulus,line2);
        	if(!Neuronmodel.good())
        	    break;
        	stringstream lineref2(line2);
	       	for(j=0;j<4;j++){
        	    getline(lineref2,strval2,',');
	      	    stringstream str2num(strval2);
		    if(j==0){str2num>>neuron[i];}
		    else if(j==1){str2num>>timestart[i];}
		    else if(j==2){str2num>>duration[i];}
		    else{str2num>>intensity[i];}
        	}
    	}
	for(i=0;i<=count;i++){
		for(j=0;j<duration[i];j++){
			I[neuron[i]][timestart[i]+j-transient]=intensity[i];
		}
	}
	

	for(i=0;i<nonodes;i++){
		Neuronmodel<<VR<<","<<VB<<","<<mu<<endl;
		Initial<<V0[i]<<","<<X0[i]<<endl;
	}
	for(i=0;i<tslength+transient;i++){
		for(j=0;j<nonodes;j++){
			Stimulus<<I[i][j];
			if(j==nonodes-1)
				Stimulus<<endl;
			else
				Stimulus<<",";
		}
	}
	
	Adjacency.close();
	Cstimulus.close();
	Neuronmodel.close();
	Initial.close();
	Stimulus.close();
	return 0;
}


//Random generator
double randomgen(void){
	double x;
	x=(double)(1.0 * rand() / (RAND_MAX + 1.0)) + 1;
	return x;
}
