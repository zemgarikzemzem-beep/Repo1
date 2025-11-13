/******************************************************************************

                              Online C++ Compiler.
               Code, Compile, Run and Debug C++ program online.
Write your code in this editor and press "Run" button to compile and execute it.

https://www.onlinegdb.com/online_c++_compiler

*******************************************************************************/

#include <iostream>
#include <fstream>

int main()
{
    const int N=512;
    int arr[N]{}; // 
    int N2, arr1;
    
std::ofstream fout("output.txt");

int j=0;

for(int i=0; i<N; ++i) arr[i]=i;

for(int i=0; i<N-2; ++i){
    if(i<j){
        arr1=arr[i]; arr[i]=arr[j]; arr[j]=arr1;
    }
    N2=N/2;
    while(N2-1<j && N2>=2){ // 
        j-=N2; N2/=2;
    }
    j+=N2;
}
    
    for(int i=0; i<N; ++i){
        fout<<arr[i]<<", ";
         if((i+1)%20==0) fout<<"\r\n";
    }

    return 0;
}