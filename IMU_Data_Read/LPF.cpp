#include "LPF.h"
#include <cmath>

void initButterworthLPF(ButterworthLPF* f, float fcHz, float fs){
  // Given cutoff calcualte coefficients to be used in actual processing

  float pi = M_PI;

  float fc = tanf((pi * fcHz) / fs);
  float norm = 1 + sqrtf(2) * fc + std::pow(fc, 2);

  f->b0 = std::pow(fc, 2) / norm;
  f->b1 = (2 * std::pow(fc, 2)) / norm;
  f->b2 = std::pow(fc, 2) / norm;

  f->a1 = (2 * (std::pow(fc, 2) - 1)) / norm;
  f->a2 = (1 - sqrtf(2) * fc + std::pow(fc, 2)) / norm;

  f->x1 = 0.0;
  f->x2 = 0.0;
  f->y1 = 0.0;
  f->y2 = 0.0;

};

float processButterworthLPF(ButterworthLPF* f, float x){

  float yn = (f->b0 * x) 
        + (f->b1 * f->x1) 
        + (f->b2 * f->x2) 
        - (f->a1 * f->y1) 
        - (f->a2 * f->y2);

  f->x2 = f->x1;
  f->x1 = x;
  f->y2 = f->y1;
  f->y1 = yn;
  return yn;

}





