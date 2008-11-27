This directory contains the script called create-this-app that builds
memtest application.  To understand the simple sequence of operations
required to build a working Nios II application, please read the 
contents of the create-this-app script.

note:
The Exception Vector of CPU was set to ssram, 
where, the Exception handler and interupt handler are located.
So, please make sure that you do not do mamtest on this location.
