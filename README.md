# At the end of

is a very simple software that blocks until all processes with a given
name have finished. It basically polls the pid of this process every
second (by default, though it can be changed) and, if nothing is captured,
the process is considered finished. This may be useful in those situations
in which we want the computer to be turned off after a long process has finished
while us not being present to do so.

For example, we may want to run a program that makes really long simulations.
However, it takes so long that it still needs, say, another two hours to complete
just right at the time we would like to go to sleep. We could do so, and leave
the computer on until we wake up in the morning and turn it off ourselves, or
contiue with our work. However, we don't need to leave the computer on. We could
tell the computer to turn itself off after some time, like this:

    $ sleep 7200; init 0     # wait two hours and turn off the computer

Of course, this is dangerous. We can never be 100% sure that the simulation
actually needs at most two hours to complete. If the _init 0_ command is called
before the simulation has finished we will loose all progress. Therefore, this is
not a good solution. Instead, we could have done the following before starting
the simulation:

    $ sudo su
    $ ./execute-simulation ; init 0

But this is what we should have done _before_ starting the simulation. Now, it
is almost complete and we don't want to start it from the very beginning.
So, we decide to use _aeo_. In another terminal we write:

    $ sudo su
    $ ./aeo -p simulation ; init 0

In this scenario, _aeo_ will block until the simulation finishes, and only after
the _init 0_ command will be invoked.

Needless to say, this is most useful in those situations in which we do not want
to create files under _sudo_, because then we will administrator permissions to edit
those files. And it may be rather tedious to change the permissions on all files
created. Also, it is very useful for forgetful people.

# Usage

The complete usage can be seen using option -h:

    $ aeo -h

The interval time used to check the status of the process can also be specified.
For example, to use a period of 0.5 seconds:

    $ aeo -p apt-get -i 0.5

The order of the options does not matter. The format of the decimal number must
conform to the format described in the manual page of the C function: strtod.
See https://linux.die.net/man/3/strtod for the documentation.
