/* NEURONS This program creates a time-series of a simple hypothesis for coupled neurons based on Galves-Locherbach stochastic model.

INPUT FILES:
- Adjacency matrix: adjacency.csv
- Neuron input parameters (VR,VB,mu): neuronmodel.csv
- Initial conditions: initial.csv
- External stimulii: stimulus.csv
OUTPUT FILES:
- Neuron firing result: timeseries.csv
- Membrane potential result: membranepotentialresults.csv
- Probabilities of neuron firing: probs.csv,
- Parameters used in the calculaton: parameters.csv

(C)Arthur Valencio 4 July 2018
Update 25 July 2018

TO DO: Read Action Potential to adopt reasonable values of neuron input parameters:
https://en.wikipedia.org/wiki/Action_potential


THIS CODE IS NOT YET OPTIMISED: 
1 - It can easily consume many Gb memory because I'm creating/loading the whole time in the 
memory, but only two points (present and immediate past) are enough.
2 - Use of full adjacency matrix is not optimal. Could make better use of sparcity.
3 - Output results could be stored in the compressed convention, which should also be faster
to process.
4 - No parallelisation yet. As neurons are independent at time t this could make computation
much faster.
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

//prototypes for Galves-Locherbach model, using special case by Ariadne, Ludmila, Roque and Stolfi
double calcprobperc(double); //calculates 
double randomgen(void);
int coinflip(double);

//Calculate the membrane potential
double potential(double V, double VB, double I, double X[], double W[][100000], double mu, double VR, int i, int nonodes){	
	double Vfuture;
	if(X[i]==1){
		Vfuture=VR;
	}
	else{
	double sumterm=0;
	int j;
	for(j=1;j<nonodes;j++){
		sumterm=sumterm+W[i][j]*X[j];
	}
	Vfuture=mu*(V-VB)+VB+I+sumterm;
	}
	return Vfuture;
}


//Main: whole network calculation
int main(void){
	//Opening input/output files
	ifstream Adjacency;
	ifstream Neuronmodel;
	ifstream Initial;
	ifstream Stimulus;
	ofstream myOutput;
	ofstream myResults;
	ofstream myPotential;
	ofstream myProbabilities;
	Adjacency.open("adjacency.csv");
	myOutput.open("parameters.csv");
	myProbabilities.open("probs.csv");
	myPotential.open("membranepotentialresults.csv");
	myResults.open("timeseries.csv");
	if(Adjacency.fail()){
		cout<<"Couldn't open file adjacency.csv"<< endl;
		exit(1);
	}	
	//Getting the parameters from user
	int i,j,t,transient,tslength,nonodes=1;
	string line, strval, line2, line3, line4, line5, strval2, strval3, strval4;	
	cout<<"Inform the time-series length: ";	
	cin>>tslength;
	cout<<"Inform the transient: ";
	cin>>transient;
	myOutput<<"Transient, "<<transient<<endl;
	myOutput<<"Time-series length, "<<tslength<<endl;
	getline(Adjacency,line);
	for(unsigned int l=0;l<line.size();l++){
			if(line[l]==','){
				nonodes=nonodes+1;
			}
	}
	double A[nonodes][nonodes]={0},V[nonodes][tslength+transient]={0};
	double x[nonodes][tslength+transient]={0},I[nonodes][tslength+transient]={0};
	double P[nonodes][tslength+transient]={0};
	double VR[nonodes]={0},VB[nonodes]={0},mu[nonodes]={0};
	double row[nonodes]={0},sumterm=0;;
	Adjacency.close();
	Adjacency.open("adjacency.csv");
	if(Adjacency.fail()){
		cout<<"Couldn't open file adjacency.csv"<< endl;
		exit(1);
	}
	for(i=0;i<nonodes;i++){
        	getline(Adjacency,line2);
        	if(!Adjacency.good())
        	    break;
        	stringstream lineref(line2);
	       	for(j=0;j<nonodes;j++){
        	    getline(lineref,strval,',');
	      	    stringstream str2num(strval);
        	    str2num>>A[i][j];
        	}
    	}
	myOutput<<"Number of nodes, "<<nonodes<<endl;
	myOutput<<"Adjacency matrix:, "<<endl;
	for(i=0;i<nonodes;i++){
		for(j=0;j<nonodes;j++){
			myOutput<<A[i][j];
			if(j+1<nonodes){
				myOutput<<',';
				cout<<' ';
			}
			else{
				myOutput<<endl;
				cout<<endl;
			}
		}
	}
	Adjacency.close();
	Neuronmodel.open("neuronmodel.csv");
	if(Neuronmodel.fail()){
		cout<<"Couldn't open file neuronmodel.csv"<< endl;
		exit(1);
	}
	myOutput<<"Neuron Model:"<<endl;
	myOutput<<"Neuron ID, VR, VB, mu"<<endl;
	for(i=0;i<nonodes;i++){
        	getline(Neuronmodel,line3);
        	if(!Neuronmodel.good())
        	    break;
        	stringstream lineref3(line3);
	       	for(j=0;j<3;j++){
        	    getline(lineref3,strval2,',');
	      	    stringstream str2num(strval2);
		    if(j==0){str2num>>VR[i];}
		    else if(j==1){str2num>>VB[i];}
		    else{str2num>>mu[i];}		
        	}
    	}
	for(i=0;i<nonodes;i++){
		myOutput<<i<<","<<VR[i]<<","<<VB[i]<<","<<mu[i]<<endl;
	}
	Neuronmodel.close();
	Initial.open("initial.csv");
	if(Initial.fail()){
		cout<<"Couldn't open file initial.csv"<< endl;
		exit(1);
	}
	myOutput<<"Initial conditions:"<<endl;
	myOutput<<"Neuron ID, V(t=0), X(t=0)"<<endl;
	for(i=0;i<nonodes;i++){
        	getline(Initial,line4);
        	if(!Neuronmodel.good())
        	    break;
        	stringstream lineref4(line4);
	       	for(j=0;j<2;j++){
        	    getline(lineref4,strval3,',');
	      	    stringstream str2num(strval3);
		    if(j==0){str2num>>V[i][0];}
		    else{str2num>>x[i][0];}		
        	}
    	}
	for(i=0;i<nonodes;i++){
		myOutput<<i<<","<<V[i][0]<<","<<x[i][0]<<endl;
	}
	Initial.close();
	int node;
	/*
	int neighb;
	//identifying degrees of each node
	int degree[nonodes];
	for(node=1;node<=nonodes;node++){	
		degree[node]=0;		
		for(neighb=1;neighb<=nonodes;neighb++){
			if(A[neighb][node]!=0){		
				degree[node]=degree[node]+1;
			}
		}
	}
	*/
	cout<<"Network configuration loaded"<<endl;
	cout<<"Reading external stimulii"<<endl;
	
	myOutput<<"External Stimulii:, "<<endl;
	Stimulus.open("stimulus.csv");
	if(Stimulus.fail()){
		cout<<"Couldn't open file stimulus.csv"<< endl;
		exit(1);
	}
	for(t=0;t<tslength+transient;t++){
        	getline(Stimulus,line5);
        	if(!Stimulus.good())
        	    break;
        	stringstream lineref5(line5);
	       	for(i=0;i<nonodes;i++){
        	    getline(lineref5,strval4,',');
	      	    stringstream str2num(strval4);
        	    str2num>>I[i][t];
        	}
    	}
	myOutput<<"t(vert)/neuronID(horiz),";
	for(i=0;i<nonodes;i++){myOutput<<i;}
	myOutput<<endl;
	for(t=0;t<tslength+transient;t++){
		myOutput<<t;
		for(i=0;i<nonodes;i++){
			myOutput<<I[i][t];
			if(i+1<nonodes){
				myOutput<<',';
				cout<<' ';
			}
			else{
				myOutput<<endl;
				cout<<endl;
			}
		}
	}
	Stimulus.close();
	cout<<"Reading complete. Proceeding to calculations."<<endl;
	myOutput.close();

	//Calculate
	for(t=1;t<tslength+transient;t++){	
		for(node=1;node<=nonodes;node++){
			if(x[node][t-1]==1){
				x[node][t]=0;
				V[node][t]=VR[node];
			}
			else{
				for(i=1;i<=nonodes;i++){				
					row[i]=x[i][t];
				}
				//caculation of potential
				if(x[node][t]==1){
					V[node][t]=VR[node];
				}
				else{
					sumterm=0;
					for(j=1;j<nonodes;j++){
						sumterm=sumterm+A[node][j]*x[j][t-1];
					}
					V[node][t]=mu[node]*(V[node][t-1]-VB[node])+VB[node]+I[node][t]+sumterm;
				}
				P[node][t]=calcprobperc(V[node][t]);
				x[node][t]=coinflip(P[node][t]);
			}
		}	
	}
	cout<<"Calculation done! Saving results to file."<<endl;
	//save
	for(t=transient;t<tslength+transient;t++){	
		for(node=1;node<=nonodes;node++){
			if(node==nonodes){
				myResults<<x[node][t]<<endl;
				myPotential<<V[node][t]<<endl;
				myProbabilities<<P[node][t]<<endl;
			}
			else{
				myResults<<x[node][t]<<',';
				myPotential<<V[node][t]<<',';
				myProbabilities<<P[node][t]<<',';
			}
		}
	}	
	myResults.close();
	myPotential.close();
	myProbabilities.close();
	cout<<"Saving complete! Check results on timeseries.csv"<<endl;
	return 0;
}



//Calculate the Probability percentage of firing
double calcprobperc(double V){
	double phi=0.5+0.5*erf(V);
	return phi;	
}



//Calculates if neuron fired or not, given the probability
int coinflip(double p){
	int x;
	double r=(double)(rand() / (RAND_MAX + 1.0)) + 1;
	if(r<=p){
		x=1;
	}
	else{
		x=0;
	}
	return x;
}
