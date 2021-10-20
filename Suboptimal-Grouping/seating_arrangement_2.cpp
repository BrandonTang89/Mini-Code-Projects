#include <iostream>
#include <algorithm>
#include <vector>
#include <fstream>
#include <cmath>
#include <map>
#include <cstring>
#include <bitset>
#include <ctime>
using namespace std;

int n, e, u, v, rating,total_rating, bit_power, counter, pax_per_group;
double weight, w, max_score = 0;
double adjmat[10000][10000];
int max_parent[10000];

vector<pair<int,int>> friends;
vector<pair<double, pair<int,int>>> edgelist; 							//weight, (u, v)

//Custom comparator
bool comp(pair<double, pair<int,int>> a, pair<double, pair<int,int>>b){
	if (a.first > b.first)return true;
	return false;
}
//UFDS
int parent[10000];
int root(int x){
	if (parent[x] == -1)return x;
	return parent[x] = root(parent[x]);
}
bool same_set(int a, int b){
	return (root(a) == root(b));
}
void connect(int a, int b){
	int roota = root(a);
	int rootb = root(b);
	if (roota == rootb)return;
	parent[roota] = rootb;
}
int group_pax[10000];
int main(){
	ios_base::sync_with_stdio(false);cin.tie(NULL);
	ifstream in("input.in");
	
	//Parsing Input
	in >> n >> pax_per_group;
	bit_power= int (26.5754247591 - 2*log2(n)) +1;							//Target of about 100 million operations (3 seconds of calculation)
	for (int i=0;i<n;i++){
		friends.clear();
		total_rating = 0;
		in >> u >> e;
		
		for (int j=0;j<e;j++){
			in >> v >> rating;
			friends.push_back(make_pair(v,rating));
			total_rating += rating;
		}
		
		//Initialising Adjmat
		weight = 1.0/double(total_rating);
		for (auto f: friends){
			adjmat[u][f.first] = weight*f.second;
		}
		
	}
	
	//Initialist Edge List
	for (int i=0; i<n;i++){
		group_pax[i] = 1;
		for (int j=i+1;j<n;j++){										//to not repeat pairs, start from i+1 (also avoids i == j)
			weight = adjmat[i][j] + adjmat[j][i];
			edgelist.push_back(make_pair(weight, make_pair(i, j)));
		}
	}
	
	sort(edgelist.begin(), edgelist.end(), comp);
	
	//MAIN
	cout << "Begining Calculations..." <<endl;
	clock_t start = clock();
	for (int i=0;i< 1<<bit_power; i++){
		
		memset(parent, -1, sizeof(parent));
		for(int j=0;j<n;j++)group_pax[j]= 1;
		bitset<28> y(i);
		
		double score = 0.0;
		counter = 0;
		
		for (auto e: edgelist){
			u = e.second.first;
			v = e.second.second;
			
			w = e.first;
			int ngp = group_pax[root(u)] + group_pax[root(v)];			//new group pax
			
			if (same_set(u, v))score += w;								//if already in same group, add to score
			if (same_set(u, v) ||ngp > pax_per_group)continue; 			//cannot compress
			
			counter ++;										   			//this edge could be skipped for optimisation
			if (counter <= bit_power && (i&(1<<counter)) > 0)continue;  //bit mask filter
			
			connect(u, v);
			score += w;													//once connected, score added
			group_pax[root(u)] = ngp;
		}
		
		//cout << "Permutation " << y<< ", score : " <<score <<endl;
		if (score > max_score){
			max_score = score;
			copy(parent, parent+n, max_parent);
		}
	}
	
	copy(max_parent, max_parent+n, parent);
	//Output (console)
	cout << "\n--- Calculations Finished --- "<<endl;
	cout << "Time Taken: " << (clock()-start) /(double) CLOCKS_PER_SEC <<endl;
	cout << "N: " << n << ", E: " << edgelist.size () <<endl;
	cout << "Permutations tested: " << (1 << bit_power) <<", bit mask power: " << bit_power<<endl;
	cout << "Max Score " << max_score <<endl;
	cout << "person:";
	for (int i=0;i<n;i++){
		cout << i << " ";
	}
	cout << endl;
	
	map<int, int>m;
	counter = 1;
	cout << "group :";
	for (int i=0;i<n;i++){
		if (m.find(root(i)) == m.end())m[root(i)] = counter++;
		cout << m[root(i)] << " ";
	}
	cout << endl;
	cout << "end" << endl;
	
	
	//Output (file)
	ofstream out("output.out");
	for (int i =1; i<counter; i++){
		out << "-" << i << endl;
		for (int j=0;j<n;j++){
			if (m[root(j)] == i)out << j << endl;
		}
	}
	
}


/*Input Format

n
u e {v rating}*e //n lines

e.g.
6
0 0
1 0
2 1 1 1
3 1 0 1
4 2 1 2 2 1
5 1 1 1
*/
	
	
	
	
	
	
	
