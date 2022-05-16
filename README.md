# BlockHammer-optimised
Implimentaion of BlockHammer with an optimized Attack Throttler.

To use, pasted these files in the src folder in the Ramultor simulator ( memory controller would be replaced)

<H2>How to use</H2>
in BlockHammer.cpp:
To deactivate the entire block hammer modulde uncomment line 287.
To deactivate the attack throttler, it can be set on passive by uncommenting line 245.
To disable just the optimization in the attack throttler uncomment line 225.

in BlockHammer.h:
Physical valuses data like Nbl diffrent time windows etc. can be changed at the top of the consturctor at the end of the file.
