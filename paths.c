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

** File: paths.c
**
** Description
**
**      This program can be used to obtain the path to a file specified as an
** argument to the command. It will search a specified environment variable if
** it exists, or paths specified on the command line
**
** Modifications
**
** 2026-05-03 John Howie        Original.
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
# include <sys/param.h>

int     main (int argc, char *argv []);
int     findfile (const char *envvarorpaths, const char *filename, bool displayallfiles);
void    DisplayHelp (void);
void    OpenInputOrOutputFile (int fd, const char *InputOrOutputFile);

extern int errno;

int main (int argc, char *argv [])
{
        char    c;
        bool    displayallfiles = false;

        // Process the command line arguments

        while ((c = getopt (argc, argv, "ah")) != -1) {
                // Process the argument

                switch (c) {
                        case 'a':
                                // Display all locations with the specified
                                // filename, not just the first

                                displayallfiles = true;
                                break;

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

        if (argc != 2) {
                // We need exactly two arguments. All we can do is display a
                // help message and exit

                DisplayHelp ();
                exit (-1);
        }

        // Call the function that converts the separator in the input file to
        // those we want in the output file

        return findfile (argv [0], argv [1], displayallfiles);
}

/* void DisplayHelp (void)
**
** This function is called do display some basic information for the user, when
** they request it or call this program incorrectly
*/

void DisplayHelp (void)
{
        printf ("Usage:\n\n");
        printf ("\tfindfile [-a] [-h] <environment variable or paths> <filename>\n");
        printf ("\n");
        printf ("This program prints the path to the specified filename. The first argument is\n");
        printf ("the name of an environment variable that contains the paths or can be the paths\n");
        printf ("themselves. The paths should be separated by colons (':').\n");
        printf ("\n");
        printf ("By default, only the first path to the filename is returned. If the -a option is\n");
        printf ("used, all locations are returned.\n");
}

/* int findfile (char *envvarorpaths, const char *file, bool displayallfiles)
**
** This function reads from the standard input (which could be a file or piped
** input). It looks for CR LF and translates it to LF when the destination is
** UNIX, and looks for LF and translates it to CR LF when the destination is
** MS-DOS
*/

int findfile (const char *envvarorpaths, const char *filename, bool displayallfiles)
{
        char *envpaths, *paths, *pathtocheck, fullpathname [MAXPATHLEN +1];
        bool foundfile = false;

        // Check if the first argument is an environment variable, by trying
        // to retrieve it

        envpaths = getenv (envvarorpaths);
        if (envpaths != (char *) 0) {
                // We found the environment variable. We need to allocate space
                // to store the paths in, as we are not meant to modify the
                // data in the environment variable

                paths = malloc (strlen (envpaths) +1);
                strcpy (paths, envpaths);
        }
        else {
                // There was no environment variable, so we assume that the
                // first argument are the paths

                paths = malloc (strlen (envvarorpaths) +1);
                strcpy (paths, envvarorpaths);
        }

        // Go through the paths, obtaining each, to check if the file exists
        // at that path

        pathtocheck = strtok (paths, ":");
        while (pathtocheck != (char *) 0) {
                // Build the full path name we check

                strcpy (fullpathname, pathtocheck);
                if (pathtocheck [strlen (pathtocheck) -1] != '/') {
                        // Append a slash to the end of the path

                        strcat (fullpathname, "/");
                }
                strcat (fullpathname, filename);

                // Check if the file exists at the full path name location

                if (! access (fullpathname, F_OK)) {
                        // We found the file! Display the path and break from
                        // the loop unless we want to get all the paths

                        printf ("%s\n", fullpathname);
                        foundfile = true;
                        if (! displayallfiles)
                                break;
                }

                // Get the next path to check

                pathtocheck = strtok ((char *) 0, ":");
        }

        // Return 0 if we found the file, and 1 if we did not

        return (foundfile ? 0 : 1);
}
