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

** File: xsv2xsv.c
**
** Description
**
**      This module takes an input file and creates an output file, line-by-
** line. It replaces one separator with another. The input separator is either
** a comma or a tab, and the output is a tab or a comma (replacing one for the
** other). Which one is replaced depends how the program is invoked. If the
** program is called as csv2tsv commas are replaced by tabs, and is tsv2csv,
** tabs are replaced by commas.
**
** The program is smart enough to look for quoted separators on the input
** stream, and ignores them (i.e. does not replace them with the output
** separator).
**
** Modifications
**
** 2026-04-25 John Howie        Original.
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
int     xsv2xsv (int isv, int osv, int quote);
void    DisplayHelp (void);
void    OpenInputOrOutputFile (int fd, const char *InputOrOutputFile);

extern int errno;

int main (int argc, char *argv [])
{
        int     isv = 0, osv = 0, quote = (int) '"';
        char    c;
        int     retval;

        // Process the command line. We start with the name of the program
        // invoked. We use it to set the input and output separator values

        if (! strcmp (argv [0], "csv2tsv")) {
                // We are converting comma-separated values input to tab-
                // separated values output

                isv = (int) ',';
                osv = (int) '\t';
        } else if (! strcmp (argv [0], "tsv2csv")) {
                // We are converting tab-separated values input to comma-
                // separated values output

                isv = (int) '\t';
                osv = (int) ',';
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

                        case 'i':
                                // The user wants to specify the input
                                // separator value

                                isv = (int) *optarg;
                                break;

                        case 'o':
                                // The user wants to specify the output
                                // separator value

                                osv = (int) *optarg;
                                break;

                        case 'q':
                                // The user wants to specify the quote
                                // character value
                               
                                quote = (int) *optarg;
                                break;

                        default:
                                // We got a flag we do not recognize, so
                                // display help and exit

                                DisplayHelp ();
                                exit (-1);
                                break;
                }
        }

        // Make sure the isv and osv values are set. There is the possibility
        // that they are not, and we cannot proceed if they are not

        if ((isv == 0) || (osv == 0)) {
                // We are missing an isv or an osv, so display help and exit

                DisplayHelp ();
                exit (-1);
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

        retval = xsv2xsv (isv, osv, quote);
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
        printf ("\tcsv2tsv [-q quote] [input] [output]\n");
        printf ("\ttsv2csv [-q quote] [input] [output]\n");
        printf ("\txsv2xsv -i <isv> -o <osv> [-q <quote>] [input] [output]\n");
        printf ("\n");
        printf ("This program converts comma-separated values input to tab-separated values\n");
        printf ("output, and vice-versa. The third form of the command allows you to use custom\n");
        printf ("separators for input and output streams. You can use the -q flag to change the\n");
        printf ("default quote character (which is '\"'). If only one filename is provided it is\n");
        printf ("assumed to be an input file.\n");
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

/* int xsv2xsv (int isv, int osv, int quote)
**
** This function reads from the standard input (which could be a file or piped
** input). It looks for unquoted input separator characters, and writes to the
** standard output (which could be a file or piped output or the terminal). The
** input and output separators, and the quote characters, are all specified as
** arguments
*/

int xsv2xsv (int isv, int osv, int quote)
{
        char    *linetoconvert = (char *) 0;
        size_t  linesize = 0;
        bool    inquotedfield = false;
        int     linecount = 0, charpos, linelength;

         // Go through the standard input, reading lines

        while ((linelength = getline (&linetoconvert, &linesize, stdin)) > 0) {
                // Update the line count

                linecount ++;

                // Go through each character in the line, looking for quote and
                // input separator characters

                for (charpos = 0; charpos < linelength; charpos ++) {
                        // Check if we have a quote or input separator at this
                        // character position. We do not need an if..else if...
                        // structure, but it makes it clear that it is isv or
                        // quote we care about (and not both), and it saves a
                        // second check when we find the isv

                        if ((int) linetoconvert [charpos] == isv) {
                                // We found the input separator value. Check if
                                // we are in a quoted field. If we are we do
                                // nothing, otherwise we replace the isv with
                                // the osv

                                if (! inquotedfield)
                                        linetoconvert [charpos] = (char) osv;
                        }
                        else if ((int) linetoconvert [charpos] == quote) {        
                                // We found the quote character. Check to see
                                // if we are in a quoted field, and proceed
                                // accordingly

                                if (! inquotedfield) {
                                        // Simple - we are not in a quoted
                                        // field so we set the flag and
                                        // continue processing

                                        inquotedfield = true;
                                }
                                else {
                                        // We are in a quoted field, so check
                                        // if this is either a quoted quote or
                                        // the end quote for the field

                                        if ((int) linetoconvert [charpos +1] == quote) {
                                                // This is a quoted quote, so
                                                // we are still in a quoted
                                                // field. Just update the
                                                // character position to skip
                                                // over this 

                                                charpos ++;
                                        }
                                        else {
                                                // We have found the end quote
                                                // for a field, so just clear
                                                // the flag

                                                inquotedfield = false;
                                        }
                                }
                        }
                }

                // Write the line out to stdout

                printf ("%s", linetoconvert);
        }

        // We are done, so just return

        return 0;
}
