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

# include "iohandler.h"

int     main (int argc, char *argv []);
int     convertfile (bool destisunix, bool utfhandling, bool ignorequotedcrlf, bool noappendcrlf);
void    DisplayHelp (void);

extern int errno;

int main (int argc, char *argv [])
{
        bool    destisunix, utfhandling = true, ignorequotedcrlf = false, noappendcrlf = false;
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

        while ((c = getopt (argc, argv, "ahqu")) != -1) {
                // Process the argument

                switch (c) {
                        case 'a':
                                // The user wants us to ensure there is a CR LF
                                // (when going from UNIX to MS-DOS) or LF (when
                                // going from MS-DOS to UNIX) at the end of the
                                // output stream

                                noappendcrlf = true;
                                break;
                        
                        case 'h':
                                // The user wants to get help, so display help
                                // and exit

                                DisplayHelp ();
                                exit (0);
                                break;

                        case 'q':
                                // The user wants to ignore CRLF or LF
                                // contained in quotes

                                ignorequotedcrlf = true;
                                break;

                        case 'u':
                                // The user wants us to leave in the UTF-8
                                // encoding characters at the beginning of
                                // a file being converted to UNIX from DOS, or
                                // not add them to a file being converted to
                                // DOS from UNIX

                                utfhandling = false;
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
                        // An error occurred, and we could not redirtet stdin

                        perror ("redirect stdin");
                        exit errno;
                }
        }

        if (argc == 2) {
                // We also need to open an output file

                if (! OpenInputOrOutputFile (1, argv [1])) {
                        // An error occurred, and we could not redirect stdout

                        perror ("redirect stdout");
                        exit errno;
                }
        }

        // Call the function that converts the separator in the input file to
        // those we want in the output file

        retval = convertfile (destisunix, utfhandling, ignorequotedcrlf, noappendcrlf);
        if (retval == -1) {
                // An error occurred. Display an error message and exit

                perror ("Unable to convert file");
                exit (EXIT_FAILURE);
        }

        // Just return success

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
        printf ("\tdos2unix [-a] [-h] [-q] [-u] [input] [output]\n");
        printf ("\tunix2dos [-a] [-h] [-q] [-u] [input] [output]\n");
        printf ("\n");
        printf ("This program converts files from MS-DOS format to UNIX format. The only changes\n");
        printf ("made are to convert CR LF to LF when moving from MS-DOS to UNIX, and LF to CR LF\n");
        printf ("when moving from UNIX to MS-DOS.\n");
        printf ("\n");
        printf ("Ordinarily, the UTF-8 prefix (the characters 0xEF 0xBB 0xBF), if present, are\n");
        printf ("removed when converting from MS-DOS to UNIX, and added, if not already present,\n");
        printf ("when converting from UNIX to MS-DOS. The -u flag reverses this behavior and\n");
        printf ("leaves the UTF-8 encoding characters untouched when converting from MS-DOS to\n");
        printf ("UNIX, and will not add them at the beginning of the output stream (if not\n");
        printf ("already present) when converting from UNIX format to MS-DOS format.\n");
        printf ("\n");
        printf ("When converting a file, any CRLF (MS-DOS) or LF (UNIX) found between quotes will\n");
        printf ("be converted. To avoid this behavior, use the -q option.\n");
        printf ("\n");
        printf ("It is guaranteed that there is a CR LF (MS-DOS) or LF (UNIX) written to the end\n");
        printf ("of the output stream (appending it as necessary). The -a flag modified this\n");
        printf ("behavior, and will not append a CR LF or LF as necessary if there is not one in\n");
        printf ("the input stream.\n");
}


/* int convertfile (bool destisunix, bool utfhandling, bool ignorequotedcrlf, bool noappendcrlf)
**
** This function reads from the standard input (which could be a file or piped
** input). It looks for CR LF and translates it to LF when the destination is
** UNIX, and looks for LF and translates it to CR LF when the destination is
** MS-DOS
*/

int convertfile (bool destisunix, bool utfhandling, bool ignorequotedcrlf, bool noappendcrlf)
{
        int     charin, linecount = 1;
        char    utfexpectedheader [3] = { 0xEF, 0xBB, 0xBF }, charcount = 0;
        bool    utfheaderfound = true, inquotes = false, striputf = false, expectlfnext = false, writetendcrlf = true;

        // Read the first three characters of the input stream, as we may need
        // to do UTF-8 processing on them. We do this in a loop, one character
        // at a time. This might seem cumbersome, but it allows us to unwind.
        // Note that we assume we found the UTF-8 header in the for loop, and
        // set it to false if we did not

        for (charcount = 0; charcount < 3 && utfheaderfound; charcount ++) {
                // Read a character from the input stream

                if ((charin = fgetc (stdin)) == EOF) {
                        // We could not read a character. Check if we have got
                        // to the end of the file

                        if (! feof (stdin)) {
                                // We are not at the end of the file. We just
                                // write out an error message and exit

                                perror ("uanble to read from stdin");
                                exit (errno);
                        }
                        else {
                                // We are at the end of the file (before we
                                // completed reading the header). We need to
                                // put what we have read so far to the standard
                                // output

                                if (fwrite ((void *) utfexpectedheader, sizeof (char), charcount, stdout) != charcount) {
                                        // We encountered an error. All we can
                                        // do is write out an error and exit

                                        perror ("unable to write to stdout");
                                        exit (errno);
                                }

                                // Just return as we have finished

                                return 0;
                        }
                }

                // Check that the character we just got is part of a UTF-8 header

                if (utfexpectedheader [charcount] != (char) charin) {
                        // We did not get the character we expected, so this is
                        // not a valid UTF-8 header. Set the flag to false so
                        // we do not loop around, again. Also, decrement the
                        // character count as it will be incremented at the
                        // end of the loop even though we have finished
                        // processing. It will accurately reflect how many
                        // valid bytes of the header we read (if any)

                        utfheaderfound = false;
                        charcount --;

                        // Push the character we just read back into the input
                        // stream. The reason we read one character at a time
                        // is that we can only guarantee that one character can
                        // be pushed back (to be honest, on a modern system that
                        // should never be an issue, but let's be safe)

                        if (ungetc (charin, stdin) == EOF) {
                                // An error occurred, and we coudl not push the
                                // character back. This is an unrecoverable
                                // error

                                perror ("unable to push character back to input stream");
                                exit (errno);
                        }
                }
         }

         // Check what direction we are going in, as that dictates what we do
         // with a UTF-8 header (if found), or a partial header
         
         if (destisunix) {
                // We are going to UNIX from MS-DOS. If we are handling UTF-8
                // we will strip the UTF-8 header from the output stream, so
                // we only write what we have if either we are NOT handling
                // UTF-8 or we do not have a UTF-8 header

                if (utfheaderfound && utfhandling) {
                        // We need to strip the header from the input stream,
                        // which is most easily done by setting the charcount
                        // to zero

                        charcount = 0;
                }
         }
         else {
                // We are going to MS-DOS from UNIX. If we are handling UTF-8
                // and there is not a UTF header, we need to write one out

                if ((! utfheaderfound) && utfhandling) {
                        // Write out a UTF-8 header to the output stream

                        if (fwrite ((void *) utfexpectedheader, sizeof (char), 3, stdout) != 3) {
                                // We were unable to write out the UTF-8
                                // handler, so write out an error message and
                                // exit as we cannot proceed

                                perror ("uanble to write UTF-8 header to output stream");
                                exit (errno);
                        }
                }
         }

         // Write out anything that we read in, which partially matched a UTF-8
         // header. This is an unlikely scenario, but we handle it just-in-case

         if (fwrite ((void *) utfexpectedheader, sizeof (char), charcount, stdout) != charcount) {
                // We were unable to write out the partial UTF-8 header (if it
                // existed), so display an error message and exit as there is
                // nothing more we can do

                perror ("unable to write partial UTF-8 header to output stream");
                exit (errno);
         }

         // Go through the standard input, reading characters. This may include
         // a character we pushed back while looking for a UTF-8 header

        while ((charin = fgetc (stdin)) != EOF) {
                // Check to see what character we got

                switch ((char) charin) {
                case '"':
                        // We got a quote character. Flip the inquotes flag so
                        // we toggle it flase to true or true to false

                        inquotes ^= true;
                        break;

                case '\r':
                        // Check to see if we are in quotes and we are ignoring
                        // quoted CRLF / LF. If we are, just break

                        if (ignorequotedcrlf && inquotes)
                                break;

                        // We found a Carriage Return (CR). Check that we are
                        // converting from MS-DOS to UNIX. If we are not we
                        // display an error message and return

                        if (! destisunix) {
                                fprintf (stderr, "Error: found CR in input in line %d\n", linecount);
                                return -1;
                        }

                        // We are going from MS-DOS to UNIX, so the next
                        // character in the input stream should be a LF. Set
                        // the flag for that

                        expectlfnext = true;
                        break;

                case '\n':
                        // Check to see if we are in quotes and we are ignoring
                        // quoted CRLF / LF. If we are, just break

                        if (ignorequotedcrlf && inquotes)
                                break;

                        // Check if we are going from MS-DOS to UNIX or UNIX to
                        // MS-DOS

                        if (destisunix) {
                                // We are going from MS-DOS to UNIX, so check
                                // that we found a preceeding CR

                                if (! expectlfnext) {
                                        // The last character read was not a CR
                                        // so display an error message and
                                        // return

                                        fprintf (stderr, "Error: found LF without preceeding CR in input in line %d\n", linecount);
                                        return -1;
                                }

                                // Clear the expectlfnext flag

                                expectlfnext = false;
                        }
                        else {
                                // We are going from UNIX to MS-DOS so we need
                                // to write out a CR

                                if (fputc ((int) '\r', stdout) == EOF) {
                                        // An error occurred, so just display
                                        // a message and exit

                                        perror ("unable to write CR to output stream");
                                        exit (errno);
                                }
                        }
 
                        // Increment the linecount

                        linecount ++;

                        // Clear the flag that says we need to write a trailing
                        // CR LF or LF

                        writetendcrlf = false;
                        break;

                default:
                        // Check that we are not expecting a LF next

                        if (expectlfnext) {
                                // An error occurred. We did not get an LF
                                // after a CR - we got some other character. We
                                // write out an error message and return

                                fprintf (stderr, "Error: missing LF after CR in input in line %d\n", linecount);
                                return -1;
                        }

                        // Set the flag that says we need to write a trailing
                        // CR LF or LF

                        writetendcrlf = true;
                        break;
                }

                // Write out the character we just read unless we are expecting
                // a LF next (if we are, we just read a CR and want to strip it)
                // out in the MS-DOS to UNIX conversion
                
                if (! expectlfnext) {
                        // Write out the character we just read

                        if (fputc (charin, stdout) == EOF) {
                                // An error occurred, so just display
                                // a message and exit

                                perror ("unable to write from input to output stream");
                                exit (errno);
                        }
                }
         }

        // Check that we are at the end of the file. If we are not, we
        // encountered a read error

        if (! feof (stdin)) {
                // We are not at the end of the file - display an error message
                // and exit
                
                perror ("read error in input stream");
                exit (errno);
        }

        // Check that we are not expecting a LF (the last character read was a
        // CR and we are going from MS-DOS to UNIX

        if (expectlfnext) {
                // We are missing a LF! Display an error message and return

                fprintf (stderr, "Error: reached end of file and missing a LF after CR\n");
                return -1;
        }
 
        // Next, see if we are in quotes and reached at the end of the file as
        // that means we probably did not handle conversions properly

        if (ignorequotedcrlf && inquotes) {
                // Display an error and return

                fprintf (stderr, "Error: reached end of input stream while in quotes\n");
                return -1;
        }

        // Check if we need to write out a trailing CR LF or LF to the file. We
        // do this if the -a flag was not used on the command line, and we did
        // not just write out the CR LF (MS-DOS) or LF (UNIX). We check the
        // last character written. If it is not a LF we know for sure we need
        // to write out CR LF or LF

        if ((! noappendcrlf) && writetendcrlf) {
                // We need to write out a CR LF or LF

                if (! destisunix) {
                        // We are going from MS-DOS to UNIX, so write out a
                        // CR (LF is written next)

                        if (fputc ((int) '\r', stdout) == EOF) {
                                // An error occurred, so display an error
                                // message and return

                                perror ("unable to write out CR LF at end of file");
                                exit (errno);
                        }
                }

                // Write out the trailing LF

                if (fputc ((int) '\n', stdout) == EOF) {
                        // An error occurred, so display an error
                        // message and return

                        perror ("unable to write out LF at end of file");
                        exit (errno);
                }
        }

        // We are done!

        return 0;
}