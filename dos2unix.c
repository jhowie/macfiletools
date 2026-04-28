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

** File: dos2unix.c
**
** Description
**
**      This module takes an input file and creates an output file, line-by-
** line. If invoked as dos2unix, it replaces CRLF with LF, and if invoked as
** unix2dos it replaces LF with CRLF
**
** Modifications
**
** 2026-04-28 John Howie        Original.
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

int     main (int argc, char *argv []);
int     convertfile (bool destisunix);
void    DisplayHelp (void);
void    OpenInputOrOutputFile (int fd, const char *InputOrOutputFile);

extern int errno;

int main (int argc, char *argv [])
{
        bool    destisunix;
        char    c;
        int     retval;

        // Process the command line. We start with the name of the program
        // invoked. We use it to set the input and output separator values

        if (! strcmp (argv [0], "dos2unix")) {
                // We are converting comma-separated values input to tab-
                // separated values output

                destisunix = true;
        } else if (! strcmp (argv [0], "unix2dos")) {
                // We are converting tab-separated values input to comma-
                // separated values output

                destisunix = false;
        }

        // Now, process the command line arguments

        while ((c = getopt (argc, argv, "hi:o:q:")) != -1) {
                // Process the argument

                switch (c) {
                        case 'h':
                                // The user wants to get help, so display help
                                // and exit

                                DisplayHelp ();
                                exit (0);
                                break;

                        default:
                                // We got a flag we do not recognize, so
                                // display help and exit

                                DisplayHelp ();
                                exit (-1);
                                break;
                }
        }

        // Update argc and argv with the flags we processed. Note that the
        // argc count is the number of remaining arguments (not including the
        // program name), and argv are the remaining arguments (not including
        // the program name)

        argc -= optind;
        argv += optind;

        // Now we need to check and see if we have other non-flag parameters,
        // which we assume to be optional input and output files. There should
        // never be more than two parameters, so check

        if (argc > 2) {
                // We have extraneous parameters. All we can do is display a
                // help message and exit

                DisplayHelp ();
                exit (-1);
        }

        if (argc >= 1) {
                // We have at least one argument. We always assume the first
                // argument is an input file

                OpenInputOrOutputFile (0, argv [0]);
        }

        if (argc == 2) {
                // We also need to open an output file

                OpenInputOrOutputFile (1, argv [1]);
        }

        // Call the function that converts the separator in the input file to
        // those we want in the output file

        retval = convertfile (destisunix);
        if (retval == -1) {
                // An error occurred. Display an error message and exit

                perror ("Unable to convert file");
                exit errno;
        }

        // Just return 0 (success)

        return 0;
}

/* void DisplayHelp (void)
**
** This function is called do display some basic information for the user, when
** they request it or call this program incorrectly
*/

void DisplayHelp (void)
{
        printf ("Usage:\n\n");
        printf ("\tdos2unix [input] [output]\n");
        printf ("\tunix2dos [input] [output]\n");
        printf ("\n");
        printf ("This program converts files from MS-DOS format to UNIX format. The only changes\n");
        printf ("made are to convert CR LF to LF when moving from MS-DOS to UNIX, and LF to CR LF\n");
        printf ("when moving from UNIX to MS-DOS.\n");
}

/* void OpenInputOrOutputFile (int fd, const char *InputOrOutputFile)
**
** This function is used to open an input or output file, and duplicate the
** file stream to the one specified (it will be either stdin or stdout)
*/

void OpenInputOrOutputFile (int fd, const char *InputOrOutputFile)
{
        int filedesc;

        // Open the file. We open it read only if the fd specified is stdin and
        // write only if it is stdout

        filedesc = open (InputOrOutputFile, ((fd == 0) ? O_RDONLY : O_WRONLY));
        if (filedesc == -1) {
                // An error occurred, and we could not open the input
                // file so display an error and exit

                perror (InputOrOutputFile);
                exit (errno);
        }

        // Duplicate the specified file descriptor

        if (dup2 (filedesc, fd) == -1) {
                // An error occurred and we could not duplicate the
                // descriptor to stdin or stdout

                perror (InputOrOutputFile);
                exit (errno);
        }                
}

/* int convertfile (bool destisunix)
**
** This function reads from the standard input (which could be a file or piped
** input). It looks for CR LF and translates it to LF when the destination is
** UNIX, and looks for LF and translates it to CR LF when the destination is
** MS-DOS
*/

int convertfile (bool destisunix)
{
        char    *linetoconvert = (char *) 0;
        size_t  linesize = 0;
        int     linelength, linecount = 0;
        bool    malformedline;

         // Go through the standard input, reading lines

        while ((linelength = getline (&linetoconvert, &linesize, stdin)) > 0) {
                // Update the linecount and assume that the line is not
                // malformed (terminated with CR LF for MS-DOS files, and LF
                // for UNIX files)

                linecount ++;
                malformedline = false;

                // Check which direction we are going in

                if (destisunix) {
                        // We are translating from MS-DOS to UNIX, so check
                        // that the second last character is CR

                        if (linetoconvert [linelength -2] != '\r') {
                                // We have a malformed line, set the flag

                                malformedline = true;
                        }
                }
                        
                // Check that the last character is LF (always true for MS-DOS
                // and UNIX files)

                if (linetoconvert [linelength -1] != '\n') {
                        // We have a malformed line, set the flag

                        malformedline = true;
                }

                // Check if the line is malformed

                if (malformedline) {
                        // Display an error message, and return

                        fprintf (stderr, "ERROR: line %d is missing %s.\n", linecount, (destisunix ? "CR LF" : "LF"));
                        return -1;
                }

                // Strip the CR LF in MS-DOS files and LF in UNIX files from
                // the line

                linetoconvert [(destisunix ? linelength -2 : linelength -1)] = (char) 0;

                // Write the line out to stdout

                printf ("%s%s", linetoconvert, ((destisunix) ? "\n" : "\r\n"));
        }

        // We are done, so just return

        return 0;
}
