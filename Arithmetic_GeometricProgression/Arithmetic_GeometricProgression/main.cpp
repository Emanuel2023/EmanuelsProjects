//
//  main.cpp
//  AP GP
//
//  Created by Emanuel Contreras on 11/22/22.
//
//Find Arithmetic and eometric progression
#include <algorithm>
#include <iostream>
#include <math.h> // Defines various mathematical functions.

using namespace std;

int main()
{
    int b = 0;
    int n = 0;
    int m = 0;
    int Arithmetic_Progression_Nth_Term;
    int Geometric_Progression_Nth_Term;
    int option;
    char YES_NO = '\0';
    double sum=0; //Here we set our integer values of the equation to 0 becuase we need the user to input what he/she would like to solve for. We also establish the names of the Arithmetic and Geometric Progression equations. The integer (option) is used to indicate which option we want to solve whether that be the Arithmetic Progression or the Geometric. Essentially, this portion of the code is where we are declaring and defining everything in our code.
   do
   {
    std::cout << "Select whether you want to find the Arithmetic or Geometric Progression \n";
    std::cout << "1-Arithmetic Progression \n";
    std::cout << "2-Geometric Progression \n";
    std::cin>>option; //Here is where we select what option we want to solve for, hence why option is a cin statement in this section of code.                   This portion of the code is essentially our Menu.
    
    switch(option)
    {
    case 1:
    std::cout << "\t========================================================== \n";
    std::cout << " \tYou have selected to solve for the Arithmetic Progression! \n";
    std::cout << "\t You can now input the values that you want to solve for. \n";
    std::cout << "\t========================================================== \n"; //The switch operator will allow us to switch what option we want to solve for, whether that be the Arithmetic or Geometric Progression. The case operator is in correlation with the switch operator and has the number values 1 and 2 after it to imply what progression sequence we want to solve for as presented previously in our Menu portion of the code (1-Arithmetic Progression 2-Geometric Progression).
    
    
    std::cout << "Enter the value for the first term of the Arithmetic Progression: \n";
    cin >> b;
    std::cout << "Enter the second term which is considered the common term of the Arithmetic Progression: \n";
    cin >> n;
    std::cout << "Enter how many terms you want to find for the Arithmetic Progression: \n";
    cin >> m;
    Arithmetic_Progression_Nth_Term = (b+(m-1)*n);
    std::cout << " The Arithmetic Progression sequence is: " << Arithmetic_Progression_Nth_Term;
    std ::cout << endl; //This portion of the code is where we are solving for the Arithmetic Progression. We enter values b, n, and m into the following equation (b+(m-1)*n) where b is the starting term, n is the common term, and m is the term we are trying to solve for.
    
    
            
for (int i=b; i<b+(m)*n; i+=n)
    {
        sum += i; // The "+" sign is used to add up the summation of the sequence for how many term we want to solve for.
        std::cout << i << ", " << endl;
        
    }
    std::cout << sum << " is what we get for the sum of the Arithmetic Progression \n"; //This portion of the code is where we are getting the summation of how many terms we are trying to solve for. When i=b, we can say that i will start at b. When i is less than our equation, we can say that i will go up to this equation where (m) is our last term. The (m) is considered to be separate or nested in the for loop. When i+=n, we are basically incrementing based on the difference or common term.
    
    std::cout << "Would you like to continue the program? y/n : ";
    cin >> YES_NO; //If the user input "y", the program will continue to run. If the user inputs "n", the program will quit. This part of the code is in correlation with the while loop at the bottom.
            
    break;
    
        
        
    case 2:
        std::cout << " \t======================================================== \n";
        std::cout << " \tYou have selected to solve for the Geometric Progression! \n";
        std::cout << "\t You can now input the values that you want to solve for. \n";
        std::cout << " \t======================================================== \n"; // Here, we have our second case that is in correlation with our switch operator. Notice how in the Menu portion of the code it has "2-Geometric Progression"; so if we select to solve for the Geometric Progression, we can simply press the 2 on our keyboard to solve for this progression sequence.
            
    
            
            
    std::cout << "Enter the value for the first term of the Geometric Progression: \n";
    cin >> b;
    std::cout << "Enter the second term which is considered the common term of the Geometric Progression: \n";
    cin >> n;
    std::cout << "Enter how many terms you want to find for the Geometric Progression: \n";
    cin >> m;
    Geometric_Progression_Nth_Term = b*pow(n,(m-1));
    std::cout << " The Geometric Progression sequence is : " << Geometric_Progression_Nth_Term;
    std::cout << endl; //This portion of the code is where we are solving for the Geometric Progression. Similar to that of our Arithmetic Progression portion of the code, we can enter the values b, n, and m into the following equation (b*pow(n,(m-1)), where b will be our starting term, n is our common term, and m is the term that we are trying to solve for.
            
   
            
            
    sum = 0;
    for (int i=0; i<m; i+=1)
    {
        sum += b*pow(n,i); // The "+" sign is used to add up the summation of the sequence.
        std::cout << b*pow(n,i) << ", " << endl;
        
    }
  
 std::cout << sum << " is the sum of the Geometric Progression \n"; //For this portion of the code, we are getting the running summation of the Geometric Progression where i+=1 is our incrementing term.
    
  
std::cout << "Would you like to continue the program? y/n : ";
cin >> YES_NO; //If the user input "y", the program will continue to run. If the user inputs "n", the program will quit. This part of the code is in correlation with the while loop at the bottom.
            
break;
    }
   }
    while (YES_NO == 'y');
    std::cout << "You have selected to quit the program, have a nice day! "; // This while loop is used to repeat the program if we select "y" and will quit the program if we select "n".
}





