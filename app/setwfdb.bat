@echo off
: file: setwfdb.bat	G. Moody	11 September 1989
:			Last revised:	22 November 2002

: This script sets environment variables used by WFDB applications.

: If you are already near the limit of your available environment space, you
: may not be able to set these variables successfully.  You may get an error
: while executing this script, or you may find that environment variables
: needed for other applications cannot be set.  In this case, you will need to
: expand your environment.  This can best be done by adding a command such as:
:      shell=c:\command.com /p /e:512
: in your config.sys file.  The number that follows the /e: is the size of
: the environment in bytes.  See your MS-DOS manual for further information.

: If you use the WFDB Software Package regularly, you will find it easiest to
: invoke this batch file from your autoexec.bat, so that the WFDB environment
: is set automatically whenever you reboot your PC.  In modern versions of
: MS-DOS, and under any version of MS-Windows, do this by adding
:    call c:\bin\setwfdb
: to your autoexec.bat.  Do not omit the _call_ from this command, or any
: commands that follow setwfdb in your autoexec.bat will not be executed.
: Replace c:\bin\ above by the correct path to this file if you install it
: somewhere else.

: The WFDB path, by analogy to the MS-DOS PATH variable, is a list of
: directories in which WFDB applications search for their input files.  You may
: define it directly, as the value of the WFDB environment variable, or
: indirectly, within a file named by the WFDB environment variable.  If you
: do none of these, the WFDB path is given by the value of DEFWFDB, which is
: defined in wfdblib.h at the time the WFDB library is compiled.  For most
: users, the default value may not need to be changed.

: Here is a simple example of a direct definition of the WFDB path:
:set WFDB=;c:\database
: This statement defines the WFDB path as consisting of the current directory
: -- this is represented by the empty string before the semicolon -- followed
: by the \database directory on the c: drive.  It is strongly recommended that
: you retain the initial null component, since any WFDB files that you create,
: such as annotation files, normally are written in the current directory and
: will be accessible for reading only if there is a null component in the WFDB
: path.  You may wish to create c:\database for long-term storage of database
: records that you create.  If you wish, add more directories to the list.

: Here is an example of an indirect definition of the WFDB path:
:set WFDB=@c:\database\wfdbpath.dos@
: This statement indicates that the WFDB path is specified by the contents of
: the file named between the @ characters -- c:\database\wfdbpath.dos.  This
: file should contain a list of all of the directories to be searched,
: separated by semicolons, in the same format as in the first example above.
: All of the directories should be listed on a single line, which may be
: arbitrarily long.  An indirect definition is useful for reducing the amount
: of environment space needed for the WFDB variable, and is necessary if the
: list of directories is so long that a direct definition would exceed the
: 127-character maximum length of an MS-DOS command.

: Within the WFDB path, any string of the form %Nr, where N is a digit between
: 1 and 8, is replaced by up to the first N characters of the current record
: name when a WFDB application searches for its input files.  The string %r is
: replaced by the entire record name.  Use this feature to set WFDB path
: components for the MIMIC Database, in which files associated with each record
: are kept in a directory named by the first 3 digits of the record name.  For
: example:
:set WFDB=;c:\database;d:\mimicdb;d:\mimicdb\%%3r
: In this example, the MIMIC Database records are assumed to be located on a
: CD-ROM in the d: drive.  Both the main MIMIC Database directory -- mimicdb --
: and the per-record subdirectories, represented by d:\mimicdb\%%3r above, are
: included in the WFDB path.  If you use a batch file such as this one to set
: the WFDB path directly, note that the % character must be doubled in order to
: prevent COMMAND.COM from attempting to perform its own variable substitution.
: COMMAND.COM does not interpret % characters typed at its prompt, however, so
: you should not double the % characters if you set WFDB without using a batch
: file.  If you set the WFDB path indirectly, again only a single % is
: necessary, since COMMAND.COM is not involved at all in this case.

: The WFDBCAL environment variable names a file that must be found somewhere in
: the WFDB path.  The file specifies the relative sizes of different types of
: signals, and is needed by applications such as view and wview so that signals
: of different types can be scaled appropriately for display.
set WFDBCAL=wfdbcal
