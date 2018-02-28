# At the end of

is a very simple software that blocks until all processes with a given
name have finished. It basically polls the pid of these process every
second and, if nothing is captured, the processed is considered finished.
This may be useful in those situations in which we want the computer to
turn off after a long process has finished while us not being present.

For example, some updates may take longer than we expected and, since we
can't wait until they finish to turn the computer off ourselves, we decide
to open another terminal and run:

$ sudo su

$ sleep(3600) && init 0

which will make the computer turn off after an hour. This is quite a bad
idea since these updates may even take longer (not likely, but just go with
it). Instead now we can do:

$ sudo su

$ aeo -p apt-get && init 0

which will turn off the computer only after the process apt-get has downloaded
and installed all updates.

Needless to say that this is most useful in those situations involving
processes that cannot be stopped, or processes that take so long that killing
them would make us lose all the progress (that will have to be repeated), and,
most specially, involving forgetful people: such a situation is one in which
we have to run some simulation that we know will take a few hours. We decide
to run it while we are not home, but we don't want the computer to stay on
more time that it needs to. So, how do we force it to turn itself off after
the simulation? Just type:

$ sudo su

$ ./execute-simulation ; init 0

However, we are in a hurry, so we completely forget about the second command.
What can we do after one hour of progress? Kill the process and start over, or
use the aeo?

# Usage

The complete usage can be seen using option -h:

$ aeo -h

You can specify the interval time used to check the status of the process. To use
a period of 0.5 seconds:

$ aeo -p apt-get -i 0.5

The order of the options does not matter. The format of the decimal number must
conform to the format described in the manual page of the C function: strtod.
See https://linux.die.net/man/3/strtod for the documentation.
