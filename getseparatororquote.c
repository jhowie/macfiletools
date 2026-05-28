/* 

BSD 3-Clause License

Copyright (c) 2026, John Howie (john@howieconsultinginc.com)

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

** File: getseparatororquote.c
**
** Description
**
**      This module contains common function(s) used by macfiletools.
**
** Modifications
**
** 2026-05-25 John Howie        Original.
**
*/

# include <stdio.h>
# include <stdlib.h>
# include <stdbool.h>
# include <fcntl.h>
# include <unistd.h>
# include <string.h>
# include <strings.h>
# include <sys/errno.h>

# include "getseparatororquote.h"

/* int getseparatororquote (const char *optarg)
**
** This function is used to process the input or output separator specified by
** the user on the command line, and return the separator character. This is
** necessary to process escaped characters such as '\t' (tab) and others
*/

int getseparatororquote (const char *optarg)
{
        int     returnvalue;

        // Check to make sure we actually got a string

        if (optarg == (char *) 0) {
                // We did not get a valid string, so return an error value (-1)

                return -1;
        }

        // Check the string to see what we got. If it begins with '\' we know
        // we have to convert it

        switch (optarg [0]) {
        case 0:
                // We got an empty string, this is an error so return the error
                // value (-1)

                return -1;
                break;

        case '\\':
                // We have an escaped value, so we need to check the second
                // character and process accordingly

                switch (optarg [1]) {
                case 't':
                        // It is a tab. Check that the next character is a NULL
                        // and if so, return the tab character else return an
                        // error value (-1)

                        if (optarg [2] == (char) 0) return (int) '\t';
                        else return -1;
                        break;

                case 'x':
                        // The user has specified a separator in the format
                        // \xHH where HH is the hexadecimal value of the
                        // separator. We assume that we have two hexadecimal
                        // digits and just attempt the conversion. If it is
                        // successful we return the value, otherwise we
                        // return the error value (-1)

                        if (sscanf ((optarg +2), "%x", &returnvalue) == 1) return returnvalue;
                        else return -1;
                        break;

                case '0':
                case '1':
                case '2':
                case '3':
                        // The user specified an octal number, so we need to
                        // convert it to the character value we return. We
                        // assue we have two or three octal digits, and attempt
                        // to convert them. If we are successful we return the
                        // value, otherwise we return the error value (-1)

                        if (sscanf ((optarg +1), "%o", &returnvalue) == 1) return returnvalue;
                        else return -1;
                        break;

                default:
                        // We got some value we do not recognize, so return an
                        // error value (-1)

                        return -1;
                        break;
                }
                break;
        default:
                // We got a simple character. Make sure the second character is
                // a NULL (end of string). If it is not we have an error

                if (optarg [1] != (char) 0) {
                        // We have an invalid separator, it can be only a
                        // single character. Return the error value (-1)

                        return -1;                        
                }

                // Return the character

                return (int) optarg [0];
                break;
        }

        // We should never, ever get here. Return the error value (-1)

        return -1;
}
