#include "fft.h"

#include "math.h"

void FFT(double* x, double* y, uint16_t N){ // , double* re, double* im
	uint8_t m=0;
	uint16_t lp=1, lp2;
	uint16_t n2=N;
	double arg, darg;
	
	while(n2>1){
		n2/=2;
		++m;
	}
	for(uint8_t l=1; l<=m; ++l){
		lp*=2;
		lp2=lp/2;
		darg=-3.1415926/lp2;
		arg=0;
		for(uint16_t j=1; j<=lp2; ++j){
			double c=cos(arg);
			double s=sin(arg);
			arg+=darg;
			for(uint16_t i=j; i<=N; i+=lp){
				uint16_t iw=i+lp2;
				double wr=x[iw]*c-y[iw]*s; // 
				double wi=x[iw]*s+y[iw]*c; // 
				x[iw]=x[i]-wr;
				y[iw]=y[i]-wi;
				x[i]+=wr;
				y[i]+=wi;
			}
		}
	}
	
}
