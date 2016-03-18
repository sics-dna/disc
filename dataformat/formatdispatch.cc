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

#include <string.h>
#include "format.hh"
#include "readtokens.hh"
#include "formatbinary.hh"
#include "formatdiscr.hh"
#include "formatcont.hh"
#include "formatsymbol.hh"
#include "formattime.hh"
#include "formatunknown.hh"
#include "formatdispatch.hh"


FormatSpec* format_create(TokenLink* tok)
{
  if (*tok->token == '"' && tok->next)
    tok = tok->next;
  if (!strcmp(tok->token, "binary:"))
    return new FormatSpecBinary;
  else if (!strcmp(tok->token, "discr:"))
    return new FormatSpecDiscr;
  else if (!strcmp(tok->token, "cont:"))
    return new FormatSpecCont;
  else if (!strcmp(tok->token, "symbol:"))
    return new FormatSpecSymbol;
  else if (!strcmp(tok->token, "date:") || !strcmp(tok->token, "time:") ||
           !strcmp(tok->token, "datetime:") || !strcmp(tok->token, "seconds:") ||
           !strcmp(tok->token, "millisec:"))
    return new FormatSpecDatetime;
  else 
    return new FormatSpecUnknown;
}

FormatSpec* format_create(char ch)
{
  FormatSpecDatetime* ret;
  if (ch=='b')
    return new FormatSpecBinary;
  else if (ch=='d')
    return new FormatSpecDiscr;
  else if (ch=='c')
    return new FormatSpecCont;
  else if (ch=='s')
    return new FormatSpecSymbol;
  else if (ch=='D') {
    ret = new FormatSpecDatetime;
    ret->dtform = 6;
    return ret;
  }
  else if (ch=='t'){
    ret = new FormatSpecDatetime;
    ret->dtform = 1;
    return ret;
  }
  else if (ch=='T') {
    ret = new FormatSpecDatetime;
    ret->dtform = 7;
    return ret;
  }
  else 
    return new FormatSpecUnknown;
}

FormatSpec* format_guess(char* str, int force)
{
  float val;
  int fmt;
  if (is_unknown(str) || is_posint(str)) {
    if (force)
      return new FormatSpecUnknown;
    else
      return 0;
  } else if (is_cont(str)) {
    if ((val = get_cont(str)) < 0.0 || val > 1.0)
      return new FormatSpecCont;
    else if (force)
      return new FormatSpecUnknown;
    else
      return 0;
  } else if ((fmt = check_datetime(str))) {
    FormatSpecDatetime* ret;
    ret = new FormatSpecDatetime;
    ret->dtform = fmt;
    return ret;
  } else
    return new FormatSpecSymbol;
}

FormatSpec* format_guess_unknown(class FormatSpecUnknown* ele)
{
  if (ele->unset)
    return 0;
  else if ((ele->min == 0.0) && (ele->max == 1.0))
    return new FormatSpecBinary;
  else if ((ele->min <= 1.0) &&
           (ele->min == (int) ele->min) &&
           (ele->max == (int) ele->max) &&
           (ele->max - ele->min <= ele->num))
    return new FormatSpecDiscr;
  else if ((ele->max - ele->min < 1.0) ||
           ((ele->max - ele->min >= 20) &&
            (ele->num >= 15)))
    return new FormatSpecCont;
  else if (ele->num < 15)
    return new FormatSpecSymbol;
  else
    return 0;
}

