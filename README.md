CSCE3600 Shell
==============
Group 16: George Davis, Shashi Dongur, Gary Johnson, Andrew Thomas

Git repository for CSCE3600 group 16
http://github.com/CyanBlob/shell
==============
In order to begin using the shell use the command: "YSH"

The `echo` issue has been resolved, and should work normally on (hopefully) all machines.

In order to use input redirection, follow this format: "[command] < [file to input from]"

In order to use output redirection, follow this format: "[command] > [file to output to]"

In order to use piping, follow this format: "[command to pipe from] | [command to pipe to]"

In order to use SuperBash utility, use the command "./SuperBash"
Sample input for SuperBash is included in file "in.sh"
To use "in.sh" as input for SuperBash use this format: "./SuperBash < in.sh"

In order to start a background job, add a `&` at the end of your command. The output is stored in background.log

In order to view your 1-minute and 24-hour average CPU usage, type `cpu`. Note that the shell can only take averages over the duration that it has been running, ie. to view an accurate 24-hour usage, the shell must be running and collecting data for a full 24 hours. Otherwise, it will print the average from the time the shell was started.

(Extra Linux Utility) In order to re-run the previous command, type "!!" into the terminal

