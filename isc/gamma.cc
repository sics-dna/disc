 /*
 --------------------------------------------------------------------------
 Copyright (C) 2006, 2015 SICS Swedish ICT AB

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

double lngamma(double x)
{
  static double lastx = -1.0;
  static double lastgl = 0.0;
  double tmp; 
  if (x==lastx) return lastgl;
  lastx = x;
  tmp = 2.5066282746310005*(1.000000000190015 + 76.18009172947146/(x+1.0) - 86.50532032941677/(x+2.0) + 24.01409824083091/(x+3.0) - 1.231739572450155/(x+4.0) + 0.001208650973866179/(x+5.0) - 0.000005395239384953/(x+6.0));
  return (lastgl = (x + 0.5)*log(x + 5.5) - x - 5.5 + log(tmp/x));
}

double digamma(double x)
{
  double tmp1, tmp2; 
  tmp1 = 1.000000000190015 + 76.18009172947146/x - 86.50532032941677/(x+1.0) + 24.01409824083091/(x+2.0) - 1.231739572450155/(x+3.0) + 0.001208650973866179/(x+4.0) - 0.000005395239384953/(x+5.0);
  tmp2 = 76.18009172947146/(x*x) - 86.50532032941677/((x+1.0)*(x+1.0)) + 24.01409824083091/((x+2.0)*(x+2.0)) - 1.231739572450155/((x+3.0)*(x+3.0)) + 0.001208650973866179/((x+4.0)*(x+4.0)) - 0.000005395239384953/((x+5.0)*(x+5.0));
  return log(x+4.5) + (x-0.5)/(x+4.5) - 1.0 - tmp2/tmp1;
}

double ddigamma(double x)
{
  double tmp1, tmp2, tmp3; 
  tmp1 = 1.000000000190015 + 76.18009172947146/x - 86.50532032941677/(x+1.0) + 24.01409824083091/(x+2.0) - 1.231739572450155/(x+3.0) + 0.001208650973866179/(x+4.0) - 0.000005395239384953/(x+5.0);
  tmp2 = 76.18009172947146/(x*x) - 86.50532032941677/((x+1.0)*(x+1.0)) + 24.01409824083091/((x+2.0)*(x+2.0)) - 1.231739572450155/((x+3.0)*(x+3.0)) + 0.001208650973866179/((x+4.0)*(x+4.0)) - 0.000005395239384953/((x+5.0)*(x+5.0));
  tmp3 = 76.18009172947146/(x*x*x) - 86.50532032941677/((x+1.0)*(x+1.0)*(x+1.0)) + 24.01409824083091/((x+2.0)*(x+2.0)*(x+2.0)) - 1.231739572450155/((x+3.0)*(x+3.0)*(x+3.0)) + 0.001208650973866179/((x+4.0)*(x+4.0)*(x+4.0)) - 0.000005395239384953/((x+5.0)*(x+5.0)*(x+5.0));
  return 1.0/(x+4.5) + 5.0/((x+4.5)*(x+4.5)) + (2*tmp3*tmp1 - tmp2*tmp2)/(tmp1*tmp1);
}

double incgammap_s(double x, double y)
// The incomplete gamma function P(x, y) evaluated by its series representation. 
{
  int i;
  double sum, tmp, xx;
  if (y <= 0.0) {
    return 0.0;
  } else {
    tmp = sum = 1.0/x;
    for (i=0, xx=x; fabs(tmp) > 1e-15*fabs(sum) && i<100; i++)
      xx += 1.0, tmp *= y/xx, sum += tmp;
    return sum*exp(-y + x*log(y) - lngamma(x));
  }
}

double incgammaq_cf(double x, double y)
// The incomplete gamma function Q(x, y) evaluated by its continued fraction representation.
{
  int i;
  double a, b, c, d, h;
  b = y+1.0-x;
  c = 0.0;
  d = 1.0/b;
  h = d;
  for (i=1; fabs(d-c) > 1e-15*fabs(c) && i<=100; i++) {
    a = -i*(i-x);
    b += 2.0;
    d = 1.0/(a*d+b);
    c = 1.0/(a*c+b);
    h *= d/c;
  }
  return exp(-y + x*log(y) - lngamma(x))*h;
}

double incgammap(double x, double y)
// Returns the incomplete gamma function P(x, y).
{
  if (y < 0.0 || x <= 0.0)
    return 0.0;
  if (y < x+1.0)
    return incgammap_s(x, y);
  else
    return 1.0-incgammaq_cf(x, y);
}

double incgammaq(double x, double y)
// Returns the incomplete gamma function Q(a, x) = 1-P(a, x).
{
  if (y < 0.0 || x <= 0.0)
    return 0.0;
  if (y < x+1.0)
    return 1.0-incgammap_s(x, y);
  else
    return incgammaq_cf(x, y);
}

