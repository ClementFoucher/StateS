
*******************
* StateS v. 0.3.H *
*******************

This software is intended to be a simple and effective discrete event machines drawer.

I currently focus on Finite State Machines (FSMs) in order to build the platform bases, but I plan to extend the software in supporting Grafcet, and perhaps someday Petri nets.

What the software can do for now:

- Draw a FSM,
- Simulate and verify it,
- Export it to VHDL (very experimental),
- Export to PDF.

You can test interacting with a machine by loading file "Demo.SfsmS".

Qt is required to use this application.


*** How to use ***


** Windows **

Download and unzip the archive, then launch StateS.exe.

Latest versions of Windows are a bit strict on security, so this is possible you obtain a security warning at opening.
Indeed, StateS is a very small application, not very well-known. Windows automatically blocks this kind of programs when downloaded from the Internet.
I guarantee there is no virus in it, so to avoid the warning, right-click on the executable, choose "Properties", check "Unlock", and hit "OK".

You can now run the application.


** Linux **

Install Qt from your distribution repositories using your package manager, or update to the latest version if already installed. At least Qt 5.7 is required.

Make sure the StateS file is executable:
From command line, cd to the folder containing the executable, and type "chmod +x ./<name of the file>".
Or graphically, right-click on the file, hit "Properties", and search for an "Allow execution" or similar checkbox, and check it.

Run the application.


*** This is early stage of the software development. ***

If you think you found a bug in StateS, please report it at address https://sourceforge.net/p/states/tickets/
But please check known_bugs.txt file first.

Any suggestion on StateS can be done on the forum at address https://sourceforge.net/p/states/discussion/
Please only suggest improvments for implemented features while I haven't implemented all the base features yet.

And remember StateS in at early stage of development!
It is not intended to be complete, or even useful for now!


StateS code license (GNU GPL V2) can be consulted using file LICENSE.txt, or at https://www.gnu.org/licenses/
To obtain code, use command:
git clone git://git.code.sf.net/p/states/code states-code
To compile, use commands `qmake` on .pro file, then `make`
Qt 5 is needed, as well as a compiler supporting c++ 2011.
For Windows, I recommand using MinGW's version of GCC. The 32-bit version can be installed along with Qt, while the 64-bit can be installed using MSys2.

Most art used in the software is, or is based on, third-party art work released under open source licences.
Authors information and corresponding licences can be found in "art" folder.
