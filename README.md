# CS344
A repository for my OSU CS344 Operating Systems coursework. Assignment descriptions below.

### Program 1
This assignment involved writing basic bash shell scripts to compute matrix operations. The purose was to become familiar with the Unix shell, shell programming, Unit utilities, standard innput, output, and error, piplines, process ids, exit values, and signals.

### Program 2
This assignment is split up into two C programs. The first program (the "rooms program") is contained in a file named tira.buildrooms.c, which when compiled with the same name (minus the extension) and ran creates a series of files that hold descriptions of the in-game rooms and how the rooms are connected. The second program (the "game") is called tira.adventure.c and when compiled with the same name (minus the extension) and ran provides an interface for playing the game using the most recently generated rooms.

In the game, the player will begin in the "starting room" and will win the game automatically upon entering the "ending room", which causes the game to exit, displaying the path taken by the player.

During the game, the player can also enter a command that returns the current time - this functionality utilizes mutexes and multithreading.

For this assignment, we were required to use raw C (C89) and were not allowed to use the C99 standard.

### Program 3
In this assignment, I wrote my own basic shell in C, similar to bash. The shell runs command line instructions and returns results similar to other shells I have used, excluding the fancier features. The name of the shell is smallsh. It prompts for commands and runs them. The shell allows redirection of standard input and output and supports both foreground and background processes (that are controllable by the command line and using signals). The shell supports the commands `exit`, `cd` , and `status` in addition to comments that begin with the # character.

### Program 4
This assignment consists of five small programs that encrypted and decrypt information using a one-time pad-like system. It combines multi-processing code, socket-based interprocess communication, command line acccessibility using unix features, and job control. A short compilation script is included.

### Program Py
A very basic Python script for implemented for learning purposes.
