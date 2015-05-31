
*******************
* StateS v. 0.3.5 *
*******************

This software is intended to be a simple and effective discrete event machines drawer.

I currently focus on Finite State Machines (FSMs) in order to build the platform bases, but I plan to extend the software in supporting Grafcet, and perhaps someday Petri nets.

What the software can do for now:

- Draw a FSM,
- Simulate and verify it,
- Export it to VHDL (very experimental),
- Export to PDF.

Qt 5.4 is required to execute this application.

You can test interacting with a machine by loading file "Demo.SfsmS".

*** This is early stage of the software development. ***

If you think you found a bug in StateS, please report it at address https://sourceforge.net/p/states/tickets/
But please check known_bugs.txt file first.

Any suggestion on StateS can be done on the forum at address https://sourceforge.net/p/states/discussion/
Please only suggest improvments for implemented features while I haven't implemented all the base features yet.

And remember StateS in at early statges of development!
It is not intended to be complete, or even useful for now!


StateS code license (GNU GPL V2) can be consulted using file LICENSE.txt, or at https://www.gnu.org/licenses/
To obtain code, use command:
git clone git://git.code.sf.net/p/states/code states-code
To compile, use commands `qmake` on .pro file, then `make`
Qt 5.4 is needed, as well as a compiler supporting c++ 2011.

Most art used in the software is, or is based on, third-party art work released under open source licences.
Authors information and corresponding licences can be found in "art" folder.
