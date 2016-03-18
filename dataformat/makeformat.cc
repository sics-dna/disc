 /*
 --------------------------------------------------------------------------
 Copyright (C) 1996, 2012, 2015 Anders Holst <aho@sics.se> 

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
#include "format.hh"

extern char selected_separator;

int main(int argc, char** argv)
{
  char* datafile;
  char* formatfile;
  Format* format;

  if (argc != 3) {
    fprintf(stdout, "Usage: makeformat <formatfile> <datafile>\n");
    exit(0);
  }
  global_unknown_tag = "\\N";

  formatfile = *(argv+1);
  datafile = *(argv+2);
  
  format = new Format();
  format->read(formatfile);
  selected_separator = ';';
  format->construct(datafile);
  format->save(formatfile);
  delete format;
  return 0;
}
