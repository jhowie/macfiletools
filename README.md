# macfiletools
A collection of tools to manipulate files on a Mac (or *BSD UNIX)

The tools included are:

        xsv2xsv         A tool to convert a file with fields separated by a
                        special character (typically a comma or a tab),
                        replacing that special character with another. For
                        example, convert a comma-separated value file into a
                        tab-separated value file (CSV to TSV). The tool follows
                        the rules for CSV files found in RFC 4180, including
                        quoted files.

                        When built and installed, symbolic links are created
                        for two additional utilities: csv2tsv and tsv2csv.
                        These set the input file separator and output file
                        separator accordingly.

        dos2unix        A tool to convert a text file with lines ending with a
                        Carriage Return (CR) and a Line Feed (LF) to a text
                        file with lines that end with a Line Feed (LF).

                        When built and installed, a symbolic link is created
                        for the utility unix2dos, which converts a text file
                        with lines ending with a Line Feed (LF) to a text file
                        with each line ending with a Carriage Return (CR) and a
                        Line Feed (LF).

        convcrlf        A tool that transforms Carriage Return (CR) and Line
                        Feed (LF) characters in the input stream, contained in
                        quotes, into the tokens <CR> and <LF> (or tokens
                        specified by the user), and wrapped by '<' and '>'
                        characters. It can do the reverse transformation, too.

The tools xsv2xsv (and csv2tsv and tsv2csv), dos2unix (and unix2dos), and
convcrlf are all useful when dealing with data files that are maniupluated by
awk, sed, sort, etc., and do not deal with field separators embedded in quotes,
understand Carriage Returns, etc.

        paths           A tool to return the location of a file in directories
                        specified in a specified environment variable (or a
                        list of directories). The first instance of the file is
                        displayed by default, but all instances can be returned
                        if requested. The desired mode can be specified.

The tool paths is most useful when writing Bash scripts, and you do not know
which lib directory library files (especially for awk) are stored in. You can
specify all library directories in an environment variable (e.g. LIBDIR), and
invoke paths such as "awk -f `paths LIBDIR function.awk` ...". The path to the
first instance of function.awk is returned.

To build and install these tools (xsv2xsv, csv2tsv, tsv2csv, dos2unix,
unix2dos, and paths), run the following commands:

        % make
        % sudo make install

Please address all bug reports to John Howie (john@howieconsultinginc.com).

John
