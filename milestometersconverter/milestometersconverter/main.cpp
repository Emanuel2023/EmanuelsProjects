//
//  main.cpp
//  milestometersconverter
//
//  Created by Emanuel Contreras on 12/27/24.
//

#include <iostream>
#include <cmath>
using namespace std;

int main() {
    
    double miles;
    double meters;
    double fixed_equation = 1609.344;
    
    std::cout << " This program will convert miles to meters\n" << endl;
    std::cout << " Please enter how many miles we are going to convert to meters\n" << endl;
    cin >> miles;
    
    meters = miles * fixed_equation;
    
    std::cout << " The conversion from " << miles << " miles to meters is " << meters << endl;
    
    
    
    
}
