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

#include "intfloat.hh"

struct TokenLink {
  char* token;
  TokenLink* next;
};

extern const char* global_unknown_tag;

extern char global_separator_char;

TokenLink* readtokens(FILE* f);
void freetokens(TokenLink* list);
char* freezebuf();
int length(TokenLink* list);
int is_posint(const char* str);
int is_negint(const char* str);
int is_cont(const char* str);
int is_unknown(const char* str);
IF_INT get_posint(const char* str);
IF_FLOAT get_cont(const char* str);
void set_selected_separator(char sep);

