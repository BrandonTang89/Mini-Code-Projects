#include <bits/stdc++.h>
using namespace std;

const double pi = 3.14159265359;
double DtoR(double in){
	return (in * pi / 180);
}
double RtoD(double in){
	return (in *180 /pi);
}


double ub, lb;
int main(){
	cout << "For sin (ax + b) = r" << endl;
	cout << "Enter 'Upperbound' 'lowerbound':" << endl;
	cin >> lb >> ub;
	cout << "Enter a b r: ";
	double a, b,r;
	cin >> a >> b >> r;
	bool quad[5] = {0, 0, 0, 0, 0};
	set <double> S;
	
	b = DtoR(b);
	lb = DtoR(lb);
	ub = DtoR(ub);
	
	lb = lb * a + b;
	ub = ub * a + b;
	
	cout << RtoD(lb) << " < x < " << RtoD(ub) << endl;
	
	double ba = asin(r);
	ba = abs(ba);
	cout << "Basic angle alpha = " << RtoD(ba) << endl;
	
	if (r > 0){
		quad[1] = quad[2] = true;
	}else quad[3] = quad[4] = true;
	
	
	if (quad[1]){
		for (double i = ba; i < ub; i += 2 * pi){
			S.insert(i);
		}
		for(double i = ba; i > lb; i += -2 * pi)S.insert(i);
	}
	if (quad[2]){
		for (double i = pi - ba; i< ub; i += 2 *pi){
			S.insert(i);
		}
		for (double i = pi - ba; i > lb; i -= 2 *pi){
			S.insert(i);
		}
	}
	
	if (quad[3]){
		for (double i = ba + pi; i< ub; i += 2 *pi){
			S.insert(i);
		}
		for (double i = ba + pi; i > lb; i -= 2 *pi){
			S.insert(i);
		}
	}
	
	if (quad[4]){
		for (double i = 2*pi - ba; i< ub; i += 2 *pi){
			S.insert(i);
		}
		for (double i = 2* pi - ba; i > lb; i -= 2 *pi){
			S.insert(i);
		}
	}
	
	set <double> solutions2;
	for (set <double>::iterator it = S.begin(); it != S.end(); it++){
		if (*it < lb)S.erase(it);
		else
		solutions2.insert((*it - b) / a);
	}
	
	for (set <double>::iterator i = solutions2.begin(); i != solutions2.end(); i++){
		cout << RtoD(*i) << ", ";
	}
	
	return 0;
}
