#indef LPF_H
#define LPF_H

struct ButterworthLPF{

  // Coefficients
  float b0, b1, b2;
  float a1, a2;

  // States and current inputs
  float x1, x2
  float y1, y2

};

void initButterworthLPF(ButterworthLPF* f, float cutoff, float fs);

float processButterworthLPF(ButterworthLPF* f, float x);

#endif