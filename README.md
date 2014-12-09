CSCE3600
========

Git repository for CSCE3600 group 16
========
The `echo` issue has been resolved, and should work normally on (hopefully) all machines.

In order to use input redirection, follow this format: "[command] < [file to input from]"

In order to use output redirection, follow this format: "[command] > [file to output to]"

In order to use piping, follow this format: "[command to pipe from] | [command to pipe to]"

In order to use SuperBash, use the command "./SuperBash"

In order to view your 1-minute and 24-hour average CPU usage, type `cpu`. Note that the shell can only take averages over the duration that it has been running, ie. to view an accurate 24-hour usage, the shell must be running and collecting data for a full 24 hours. Otherwise, it will print the average from the time the shell was started.

Background jobs, and the superbash additions remain a work in progress, but should not require much more work. (They have not been implemented in the current build at all, as of the night of 11/25/14)
------

On a the CSE machines or your own Linux machine, run: git clone https://github.com/CyanBlob/shell.git to copy the Git directory onto your machine

To update the GitHub with your changes, run:


git add filename

git commit -m "Commit message"

git push origin


In order to pull the most recent version from the GitHub, run:

git pull origin


Tasks:

1) Echo - Fixed - Andrew

2) Output redirection - Fixed - Andrew

3) Input redirection - Fixed - Andrew (pushed it under the bed)

4) 24 hour CPU usage - Fixed - Andrew



5) Piping - Working fully - Assigned to Kane

6) Background Jobs - Assigned to Shashi

7) SuperBash - Assigned to Gary




8) New Linux Utility - Up for grabs


9) Extra utility (extra credit) - Anyone
