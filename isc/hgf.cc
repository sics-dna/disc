 /*
 --------------------------------------------------------------------------
 Copyright (C) 2011, 2015 SICS Swedish ICT AB

 Main author: Anders Holst <aho@sics.se>

 The code in this file is based on publicly published algorithms.

 This code is free software: you can redistribute it and/or modify it
 under the terms of the GNU Lesser General Public License as published
 by the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with this code.  If not, see <http://www.gnu.org/licenses/>.
 --------------------------------------------------------------------------
 */

#include <math.h>
#ifdef WIN32
#define _USE_MATH_DEFINES
#include <cmath>
#endif
#include "gamma.hh"

#define MAXITER 10000

// Gauss hypergeometric series 2F1. Only for real numbers and z < 1.

// Brute force adding up the terms of the series. Ok for small z.
double hypergeometric_brute(double a, double b, double c, double z)
{
  int n;
  double res, tmp, k;

  k = tmp = res = 1.0;
  for (n=1; n<MAXITER; n++) {
    k *= a*b/c * z/n;
    res += k;
    if (tmp == res) break;
    tmp = res;
    a += 1.0;
    b += 1.0;
    c += 1.0;
  }
  return res;
}

double hypergeometric(double a, double b, double c, double z)
{
  if (z >= 1.0) {
//    printf("hypergeometric: invalid parameters\n");
    return 0.0;
  } else if (z > -0.5)
    return hypergeometric_brute(a, b, c, z);
  else if (a > b)
    return exp(-b*log(1.0-z))*hypergeometric_brute(c-a, b, c, z/(z-1.0));
  else
    return exp(-a*log(1.0-z))*hypergeometric_brute(a, c-b, c, z/(z-1.0));
}

double loghypergeometric(double a, double b, double c, double z)
{
  if (z >= 1.0) {
//    printf("loghypergeometric: invalid parameters\n");
    return 0.0;
  } else if (z > -0.5)
    return log(hypergeometric_brute(a, b, c, z));
  else if (a > b)
    return -b*log(1.0-z)+log(hypergeometric_brute(c-a, b, c, z/(z-1.0)));
  else
    return -a*log(1.0-z)+log(hypergeometric_brute(a, c-b, c, z/(z-1.0)));
}

// Integral of the "core" student-t distribution from z to infinity, assuming z>=0
// f(z) = 1/(1+z^2)^c, normalized to 0.5
double intstudent(double c, double z)
{
  if (z<0.0) z=-z;
  if (c*z*z >= 10.0)
    return exp(lngamma(c) - lngamma(c+0.5) - log(1.0+z*z)*(c-0.5)) / (2.0*sqrt(M_PI)) *
           hypergeometric_brute(c-0.5, 0.5, c+0.5, 1.0/(1.0+z*z));
  else
    return 0.5 - exp(lngamma(c) - lngamma(c-0.5) -log(1.0+z*z)*c) / sqrt(M_PI) * z *
           hypergeometric_brute(c, 1.0, 1.5, z*z/(1.0+z*z));
}

// Logarithm of integral of student-t distribution from z to infinity, assuming z>=0
double logintstudent(double c, double z)
{
  if (z<0.0) z=-z;
  if (c*z*z >= 10.0)
    return lngamma(c) - lngamma(c+0.5) -0.5*log(4.0*M_PI) - log(1.0+z*z)*(c-0.5) +
           log(hypergeometric_brute(c-0.5, 0.5, c+0.5, 1.0/(1.0+z*z)));
  else
    return log(0.5 - exp(lngamma(c) - lngamma(c-0.5) -log(1.0+z*z)*c) / sqrt(M_PI) * z *
               hypergeometric_brute(c, 1.0, 1.5, z*z/(1.0+z*z)));
}

// Integral of a multidimensional generalization of the student-t distribution from z to infinity, assuming z>=0
// f(z) = z^d/(1+z^2)^c, normalized to 1.0
double intmultistudent(double c, double z, double d)
{
  if (z<0.0) z=-z;
  if ((c-0.5*d)*z*z >= 10.0)
    return exp(lngamma(c) - lngamma(c+0.5*(1.0-d)) - lngamma(0.5*(1.0+d)) - log(1.0+z*z)*(c-0.5*(1.0+d))) *
      hypergeometric_brute(c-0.5*(1.0+d), 0.5*(1.0-d), c+0.5*(1.0-d), 1.0/(1.0+z*z));
  else
    return 1.0 - exp(lngamma(c) - lngamma(c-0.5*(1.0+d)) - lngamma(0.5*(3.0+d)) -log(1.0+z*z)*c + log(z)*(1.0+d)) *
      hypergeometric_brute(c, 1.0, 0.5*(3.0+d), z*z/(1.0+z*z));
}

// Logarithm of integral of multidimensional student-t distribution from z to infinity, assuming z>=0
double logintmultistudent(double c, double z, double d)
{
  if (z<0.0) z=-z;
//  if ((c-0.5*d)*z*z >= 10.0 && z > 0.3)
//    return lngamma(c) - lngamma(c+0.5*(1.0-d)) - lngamma(0.5*(1.0+d)) - log(1.0+z*z)*(c-0.5*(1.0+d)) +
//      log(hypergeometric_brute(c-0.5*(1.0+d), 0.5*(1.0-d), c+0.5*(1.0-d), 1.0/(1.0+z*z)));
  if (c <= d)
    return 0.0;
  else
    return log(1.0 - exp(lngamma(c) - lngamma(c-0.5*(1.0+d)) - lngamma(0.5*(3.0+d)) -log(1.0+z*z)*c + log(z)*(1.0+d)) *
               hypergeometric_brute(c, 1.0, 0.5*(3.0+d), z*z/(1.0+z*z)));
}
