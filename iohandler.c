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

** File: iohandler.c
**
** Description
**
**      This module contains common function(s) used by macfiletools.
**
** Modifications
**
** 2026-05-16 John Howie        Original.
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

# include "iohandler.h"

/* bool OpenInputOrOutputFile (int fd, const char *InputOrOutputFile)
**
** This function is used to open an input or output file, and duplicate the
** file stream to the one specified (it will be either stdin or stdout)
*/

bool OpenInputOrOutputFile (int fd, const char *InputOrOutputFile)
{
        int filedesc;

        // Open the file. We open it read only if the fd specified is stdin and
        // write only if it is stdout

        filedesc = open (InputOrOutputFile, ((fd == 0) ? O_RDONLY : (O_CREAT | O_TRUNC | O_WRONLY)));
        if (filedesc == -1) {
                // An error occurred, and we could not open the input
                // file so display an error and exit

                return false;
        }

        // Duplicate the specified file descriptor

        if (dup2 (filedesc, fd) == -1) {
                // An error occurred and we could not duplicate the
                // descriptor to stdin or stdout

                return false;
        }     
        
        return true;
}