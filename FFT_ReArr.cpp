/******************************************************************************

                              Online C++ Compiler.
               Code, Compile, Run and Debug C++ program online.
Write your code in this editor and press "Run" button to compile and execute it.

https://www.onlinegdb.com/online_c++_compiler

*******************************************************************************/

#include <iostream>
#include <fstream>
#include <iomanip>
#include "math.h"

int main()
{
    const int logN=9;
    double arr[logN]{}; // 
    int N2, arr1;
    
    std::ofstream fout("output.txt");

    int j=0;

    for(int i=0; i<logN; ++i) arr[i]=(abs(cos(M_PI/pow(2,i)))<1e-5)?0:cos(M_PI/pow(2,i));
       
    for(int i=0; i<logN; ++i){
        fout<<std::fixed<<std::setprecision(15) <<arr[i]<<", ";
        if((i+1)%5==0) fout<<"\r\n";
    }

    return 0;
}