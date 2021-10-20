#include <bits/stdc++.h>
#include "mapping.h"
using namespace std;

int adjmat[200][200]; //Adjcency Matrix: adjmat[i][j] is time taken to get from i to j
int p[200][200]; //Parent Matrix [i][j] stores the last station (before j) visited on the path from i to j
int tas[200]; //Time at stop
string name, start, dest;

int n=0;
int e=0;
int u, v, w;
int totaltime;
ifstream input;
string station_name(int x){
	for (auto i = m.begin(); i != m.end(); i++){
		if (i->second == x)return i->first;
	}
	return ("");
};
void printpath(int i, int j){
	if (i!=j)printpath(i, p[i][j]);
	cout<< station_name(j)<<" ";
}
int main(){
	mapping();
	input.open("input.txt"); //Open input file
	for (int i=0;i<200;i++){
		for (int j=0;j<200;j++){
			if (i == j)continue;
			adjmat[i][j] = 100000;
		}
	}
	while(true){//Processing nodes (stations)
		//cin >> name;
		input >> name;
		if (name == "END")break;
		if (name == "//"){
			getline(input, name);
			//getline(cin, name);
		}
		else{
			n++;
			input >> tas[m[name]];
		}
	}
	
	cout << "Done Nodes" <<endl;
	while(true){//Processing edges (path between stations)
		//cin >>start;
		input >> start;
		if (start == "END")break;
		if (start == "//"){
			getline(input, start);
			//getline(cin, start);
		}
		else{
			input >> dest >> w;
			//cin >> dest >> w;
			e++;
			u = m[start]; v = m[dest];
			adjmat[u][v] = adjmat[v][u] = w;
		}
		
	}
	cout << "Done Edges"<<endl;
	n=137;
	input.close(); //Close the file
	for (int i=0;i<n;i++) //initialising parent matrix
		for (int j=0;j<n;j++)
			p[i][j] = i; //Each station points to itself for all stations it goes to
	

	//Floyd Warshall's All Pair Shortest Path Algorithm
	for(int k=0;k<n;k++){
		for (int i=0;i<n;i++){
			for (int j=0;j<n;j++){
				if (adjmat[i][k] + adjmat[k][j] + tas[k] < adjmat[i][j]){ //If going from i to k to j is faster than
					adjmat[i][j] = adjmat[i][k] + adjmat[k][j] + tas[k];  //going from i to j directly, go to k first
					p[i][j] = p[k][j]; 
				}
			}
		}
	}
	
	cout << "Done Processing" <<endl;
	while(1){
		cout << "Query: Start End" <<endl;
		cin >> start >> dest;
		if (start == "END")break;
		if (m.find(start) == m.end() || m.find(dest) == m.end()){
			cout << "Invalid Query" << endl;
			continue;
		}
		printpath(m[start], m[dest]);
		cout <<endl;
		totaltime =adjmat[m[start]][m[dest]];
		cout << "Total time: " << totaltime/60 << " mins and " << totaltime%60 << " seconds!(ish) " <<endl;
		cout << "-------------------" <<endl;
	}
}
