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

** File: convcrlf.c
**
** Description
**
**      This module takes an input file and creates an output file, line-by-
** line. It replaces CF LF in quotes with <<CR><LF>>, LF with <<LF>> and vice
** versa. How it does this can be controlled with command line options.
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

# include "iohandler.h"
# include "getseparatororquote.h"

int     main (int argc, char *argv []);
bool    convertfile (const char *crtoken, const char *lftoken, int quotechar, bool convfromcrlf, bool convtocrlf, bool ignorequotes);
bool    processpossibleCRLFtoken (const char *crtoken, const char *lftoken);
void    DisplayHelp (void);

extern int errno;

int main (int argc, char *argv [])
{
        int     quotechar = (int) '"';
        char    c, *crtoken = "<CR>", *lftoken = "<LF>";
        bool    ignorequotes = false, convfromcrlf = false, convtocrlf = false;
        int     retval;

        // Process the command line arguments

        while ((c = getopt (argc, argv, "c:fhil:q:t")) != -1) {
                // Process the argument

                switch (c) {
                        case 'c':
                                // The user wants to specify the string that is
                                // used to replace or search for CR

                                crtoken = optarg;
                                break;

                        case 'f':
                                // The user wants to force replacement of \r\n
                                // and \n to <<CR><LF>> and <<LF>> (or to the
                                // specified strings) respectively, but not
                                // other way
                                
                                convfromcrlf = true;
                                break;

                        case 'h':
                                // The user wants to get help, so display help
                                // and exit

                                DisplayHelp ();
                                exit (EXIT_SUCCESS);
                                break;

                        case 'i':
                                // The user wants to convert all CR LF and/or
                                // LF, even those not in quotes

                                ignorequotes = true;
                                break;

                        case 'l':
                                // The user wants to specify the string that is
                                // user to replace or search for LF

                                lftoken = optarg;
                                break;

                        case 'q':
                                // The user wants to specify the quote
                                // character value
                               
                                if ((quotechar = getseparatororquote(optarg)) == -1) {
                                        // We did not get a valid quote on the
                                        // command line, so display an error
                                        // and then the help

                                        (void) fprintf (stderr, "Invalid quote character %s\n", optarg);
                                        DisplayHelp ();
                                        exit (-1);
                                }
                                break;

                        case 't':
                                // The user wants to force replacement of
                                // <<CR><LF>> and <<LF>> (or the respective
                                // strings) to \r\n and \n, but not the other
                                // way

                                convtocrlf = true;
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

                if (! OpenInputOrOutputFile (0, argv [0])) {
                        // An error occurred, and we could not redirect stdin

                        perror ("redirect stdin");
                        exit errno;
                }
        }

        if (argc == 2) {
                // We also need to open an output file

                if (! OpenInputOrOutputFile (1, argv [1])) {
                        // An error occurred, and we coudl not redirect stdout

                        perror ("redirect stdout");
                        exit errno;
                }
        }

        // Call the function that converts the separator in the input file to
        // those we want in the output file

        retval = convertfile (crtoken, lftoken, quotechar, convfromcrlf, convtocrlf, ignorequotes);
        if (retval == false) {
                // An error occurred. Display an error message and exit

                perror ("Unable to convert file");
                exit (EXIT_FAILURE);
        }

        // Exit a success

        exit (EXIT_SUCCESS);
        return 0;               // Redundant
}

/* void DisplayHelp (void)
**
** This function is called do display some basic information for the user, when
** they request it or call this program incorrectly
*/

void DisplayHelp (void)
{
        printf ("Usage:\n\n");
        printf ("\tconvcrlf [-c CR] [-f] [-i] [-l LF] [-q quote] [-t] [input] [output]\n");
        printf ("\n");
        printf ("This program converts embedded \\r\\n (CRLF) and \\n (LF) found in quotes ('\"') in\n");
        printf ("the input stream to <<CR><LF>> and <<LF>> respectively to the output stream when\n");
        printf ("the -f flag is used, and the reverse when the -t flag is used. Use of both flags\n");
        printf ("simultaneously will cause all \\r\\n and \\n to be converted to <<CR><LF>> and\n");
        printf ("<<LF>>, and <<CR><LF>> and <<LF>> to \\r\\n and \\r simultaneoously.\n");
        printf ("\n");
        printf ("The substitution for \\r (<CR>) and \\n (<LF>) can be overridden by use of the -c\n");
        printf ("and -l flags. It is used in conversions both ways (from and to \\r\\n and \\n)\n");
        printf ("\n");
        printf ("The -i flag can be used to force the program to ignore quotes, and convert all\n");
        printf ("\\r\\n and \\n (or string equivalents) found in the input stream. The quote\n");
        printf ("character can be specified using the -q flag, and can be a character or a number\n");
        printf ("in hexadecimal (\\xHH) or octal (\\nnn)format.\n");
}

/* bool convertfile (const char *crtoken, const char *lftoken, int quotechar, bool convfromcrlf, bool convtocrlf, bool ignorequotes)
**
** This functions reads from the standard input looking for quotes (unless the
** -i flag is used) then for carriage returns "\r" and newlines "\n" and for
** <CR> (or user specified) and <LF> (or user specified) strings depending on
** the use of the -f (convfromcrlf) and -t (convtocrlf) flags.
*/

bool convertfile (const char *crtoken, const char *lftoken, int quotechar, bool convfromcrlf, bool convtocrlf, bool ignorequotes)
{
        int     charin, inspectionpos;
        bool    inquotedsection = false, foundcrlf = false;

        // We go through the input stream, character by character, looking for
        // various tokens in the input stream

        while ((charin = fgetc (stdin)) != EOF) {
                // Check what we got - see if it is a token we are looking for

                switch (charin) {
                case '<':
                        // We have possibly found the beginning of the string
                        // representation of CRLF or LF. We only care if we
                        // are in a quoted section or are ignoring quotes, and
                        // converting to \r and \n 

                        if ((inquotedsection || ignorequotes) && convtocrlf) {
                                // Call a function we use to process the tokens
                                // following this one, to see if we need to
                                // write out one or more \r and/or \n

                                if (! processpossibleCRLFtoken (crtoken, lftoken)) {
                                        // We were unable to process the
                                        // possible CRLF or LF token, so all we
                                        // can do is return false

                                        return false;
                                }
                        }
                        else {
                                // We need to write out the '<' character as it
                                // is not the start of a token
                                
                                if (fputc ((int) '<', stdout) == EOF) {
                                        // An error occurred, just return an
                                        // error

                                        return false;
                                }

                        }
                        break;

                case '\r':
                case '\n':
                        // We found a carriage return or a line feed. Check if
                        // we are in a quoted section or if we are ignoring
                        // quoted sections, and that we are converting CRLF and
                        // LF

                        if ((inquotedsection || ignorequotes) && convfromcrlf) {
                                // We need to process the carriage return or
                                // line feed. We start by checking if we are
                                // already processing CRLF / LF
                                
                                if (! foundcrlf) {
                                        // This is the first CRLF or LF

                                        foundcrlf = true;

                                        // Write out the token ('<') that we
                                        // look for in the input stream when we
                                        // convert back to carriage returns and
                                        // line feeds

                                        if (fputc ((int) '<', stdout) == EOF) {
                                                // An error occurred. There is
                                                // nothing more we can do, so
                                                // return false

                                                return false;
                                        }
                                }

                                // Write out the CR string or the LF string as
                                // appropriate

                                if (fprintf (stdout, "%s", ((charin == (int) '\n') ? lftoken : crtoken)) < 0) {
                                        // A error occurred, and we could not
                                        // write out the string representation
                                        // of the CR or LF character, so we
                                        // just return false

                                        return false;
                                }
                        }
                        else {
                                // We are not converting the carriage return
                                // or new line, so just write it out

                                if (fputc (charin, stdout) == EOF) {
                                        // An error occurred, and we can only
                                        // return false

                                        return false;
                                }
                        }
                        break;

                default:
                        // We found something other than <, \r, or \n. First
                        // thing we do is check if we were processing \r and/or
                        // \n to CR and/or LF strings, and close out the
                        // conversion by writing '>'

                        if (foundcrlf) {
                                // Write out the closing '>'

                                if (fputc ((int) '>', stdout) == EOF) {
                                        // An error occurred, and we could not
                                        // write out the end of sequence
                                        // character. All we can do is return
                                        // false

                                        return false;
                                }

                                // Clear the foundcrlf flag

                                foundcrlf = false;
                        }

                        // Check if we found the quote character. We cannot do
                        // that in the switch statement, above, as we can only
                        // check for fixed values

                        if (charin == quotechar) {
                                // We found the quote character, so flip the
                                // flag we use to track whether or not we are
                                // in a quoted section

                                inquotedsection ^= true;
                        }

                        // Write out the character we just read in

                        if (fputc (charin, stdout) == EOF) {
                                // An error occurred and we could not write out
                                // the character we just read in. All we can do
                                // is return false

                                return false;
                        }
                        break;
                }
        }

        // Check that we reached the legitimate end of the input stream, and
        // did not encounter an error

        if (! feof (stdin)) {
                // We had a read error. Just return false

                return false;
        }

        // We finished - return true

        return true;
}

/* bool processpossibleCRLFtoken (const char *crtoken, const char *lftoken)
**
** This function is called to process a possible CR or LF string token, to be
** converted into \r and \n respectively. It actually does more - it looks for
** <CR> and/or <LF> (or user-specified strings) all the way up to a '>' after
** the last token
*/

bool processpossibleCRLFtoken (const char *crtoken, const char *lftoken)
{
        int     charin, tokenpos = 0, inputpos;
        bool    possiblecr = true, possiblelf = true, writtencrlf = false;

        // We go into a loop reading characters until we find '>'. We process
        // what we find, checking for <CR> and/or <LF> tokens (or user-
        // specified tokens), and writing our \r and/or \n as required

        while ((charin = fgetc (stdin)) != EOF) {
                // Check if the character we just read in is '>'

                if (charin == (int) '>') {
                        // Check if we have just written out a \r or \n, and
                        // we have not begun processing a new token, as that
                        // means we have completed the token(s). We can just
                        // return

                        if (writtencrlf && (tokenpos == 0))
                                return true;
                }

                // Check to see if the character we read matches the next
                // expected character in the token strings <CR> or <LF> (or the
                // equivalent strings specified by the user)

                // Start with the CR token

                if (possiblecr) {
                        // Check if the character we read is the next expected
                        // character from the CR token

                        if (*(crtoken + tokenpos) == (char) charin) {
                                // Check if this is the last character in the
                                // token

                                if (*(crtoken + tokenpos +1) == (char) 0) {
                                        // We have reached the end of the CR
                                        // token, so write out the carriage
                                        // return

                                        if (fputc ((int) '\r', stdout) == EOF) {
                                                // We ran into an error, and
                                                // were unable to write out
                                                // the carriage return

                                                return false;
                                        }

                                        // Reset the the various flags and
                                        // counters we use

                                        possiblecr = true;      // Unecessary
                                        possiblelf = true;
                                        writtencrlf = true;
                                        tokenpos = 0;
                                        continue;
                                }
                        }
                        else {
                                // The character we read is not the next
                                // expected one in the token. Check if we are
                                // still looking for a LF token

                                possiblecr = false;

                                if (! possiblelf) {
                                        // We are not looking for a LF token,
                                        // so we write out what we read in the
                                        // CR token so far (and an initial '<')

                                        if (writtencrlf) {
                                                // We are in a quandary. We
                                                // successfully processed one
                                                // or more CR and/or LF tokens,
                                                // which means the input stream
                                                // is corrupted as we did not
                                                // get the trailing '>'. All we
                                                // can do is return false

                                                return false;
                                        }

                                        // Write out everything we have read
                                        // up-to this character beginning with
                                        // the '<'

                                        if (fputc ((int) '<', stdout) == EOF) {
                                                // We were unable to write out
                                                // the '<' character

                                                return false;
                                        }

                                        // Go into a loop, writing out the
                                        // characters in the CR token up to the
                                        // position of the mismatching
                                        // character

                                        for (inputpos = 0; inputpos < tokenpos; inputpos ++) {
                                                if (fputc ((int) *(crtoken + inputpos), stdout) == EOF) {
                                                        // An error occurred,
                                                        // and we were unable
                                                        // to write out the
                                                        // character. Just
                                                        // return false

                                                        return false;
                                                }
                                        }

                                        // Write out the character we just read

                                        if (fputc (charin, stdout) == EOF) {
                                                // We were unable to write out
                                                // the character we just read,
                                                // so return an error

                                                return false;
                                        }

                                        // We can return success, as we did not
                                        // find a CR or LF token, and wrote out
                                        // the characters we read instead

                                        return true;
                                }
                        }
                }

                // Now we do the same, again, for the LF token
                
                if (possiblelf) {
                        // Check if the character we read is the next expected
                        // character from the CR token

                        if (*(lftoken + tokenpos) == (char) charin) {
                                // Check if this is the last character in the
                                // token

                                if (*(lftoken + tokenpos +1) == (char) 0) {
                                        // We have reached the end of the CR
                                        // token, so write out the carriage
                                        // return

                                        if (fputc ((int) '\n', stdout) == EOF) {
                                                // We ran into an error, and
                                                // were unable to write out
                                                // the carriage return

                                                return false;
                                        }

                                        // Reset the the various flags and
                                        // counters we use

                                        possiblecr = true;
                                        possiblelf = true;      // Unecessary
                                        writtencrlf = true;
                                        tokenpos = 0;
                                        continue;
                                }
                        }
                        else {
                                // The character we read is not the next
                                // expected one in the token. Check if we are
                                // still looking for a LF token

                                possiblelf = false;

                                if (! possiblecr) {
                                        // We are not looking for a CR token,
                                        // so we write out what we read in the
                                        // LF token so far (and an initial '<')

                                        if (writtencrlf) {
                                                // We are in a quandary. We
                                                // successfully processed one
                                                // or more CR and/or LF tokens,
                                                // which means the input stream
                                                // is corrupted as we did not
                                                // get the trailing '>'. All we
                                                // can do is return false

                                                return false;
                                        }

                                        // Write out everything we have read
                                        // up-to this character beginning with
                                        // the '<'

                                        if (fputc ((int) '<', stdout) == EOF) {
                                                // We were unable to write out
                                                // the '<' character

                                                return false;
                                        }

                                        // Go into a loop, writing out the
                                        // characters in the CR token up to the
                                        // position of the mismatching
                                        // character

                                        for (inputpos = 0; inputpos < tokenpos; inputpos ++) {
                                                if (fputc ((int) *(lftoken + inputpos), stdout) == EOF) {
                                                        // An error occurred,
                                                        // and we were unable
                                                        // to write out the
                                                        // character. Just
                                                        // return false

                                                        return false;
                                                }
                                        }

                                        // Write out the character we just read

                                        if (fputc (charin, stdout) == EOF) {
                                                // We were unable to write out
                                                // the character we just read,
                                                // so return an error

                                                return false;
                                        }

                                        // We can return success, as we did not
                                        // find a CR or LF token, and wrote out
                                        // the characters we read instead

                                        return true;
                                }
                        }
                }

                // Increment tokenpos

                tokenpos ++;
        }

        // If we got here, an error occurred. It does nto matter if it is the
        // end of file or read error. We should have exited the loop, returning
        // to the caller, when we found a valid '>'

        return false;
}