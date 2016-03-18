 /*
 --------------------------------------------------------------------------
 Copyright (C) 2011, 2015 Anders Holst <aho@sics.se> 

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

#include <stdio.h>
#include <stdlib.h>
#include "readtokens.hh"

void selection(FILE* f1, FILE* f2, int n, int* vec)
{
  TokenLink *row, *tok;
  int i, j;
  while ((row = readtokens(f1))) {
    j=0;
    tok = row;
    for (i=0; i<n; i++) {
      if (!row) {
        fputs("\n", f2);
        continue;
      }
      if (vec[i] < j)
        j = 0, tok = row;
      while (j < vec[i] && tok)
        j++, tok = tok->next;
      if (tok)
        fputs(tok->token, f2);
      if (i+1<n)
        fputs("\t", f2);
    }
    fputs("\n", f2);
    freetokens(row);
  }
}

int main(int argc, char** argv)
{
  FILE *file1, *file2;
  int num, i;
  int* vec;

  if (argc <= 3) {
    fprintf(stdout, "Usage: selectcolums <infile> <outfile> <col1> ...\n");
    exit(0);
  }
  if ((file1 = fopen(*(argv+1), "r")) == 0)
    return 0;
  if ((file2 = fopen(*(argv+2), "w")) == 0)
    return 0;
  num = argc - 3;
  vec = new int[num];
  for (i=0; i<num; i++)
    vec[i] = atoi(*(argv+i+3))-1;
  global_separator_char = '\t';
  selection(file1, file2, num, vec);
  delete vec;
  fclose(file2);
  fclose(file1);
}
