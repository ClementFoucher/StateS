
*******************
* StateS v. 0.3.A *
*******************

This software is intended to be a simple and effective discrete event machines drawer.

I currently focus on Finite State Machines (FSMs) in order to build the platform bases, but I plan to extend the software in supporting Grafcet, and perhaps someday Petri nets.

What the software can do for now:

- Draw a FSM,
- Simulate and verify it,
- Export it to VHDL (very experimental),
- Export to PDF.

You can test interacting with a machine by loading file "Demo.SfsmS".


*** How to use ***


Get latest version of Qt from http://www.qt.io/download-open-source/
At least Qt 5.4 is required, so if you have a previous version you need to download a more recent one.

Run Qt installer, and make sure to check the "msvc2013 64-bit" component under the "Qt 5.5" item.
If you're running a 32 bits version of Windows, check "msvc2013 32-bit" instead.

Hit the keyboard keys "Windows" and "Pause" simulatenously, and go to "advanced system parameters" in the left column.
Hit the "Environment variables..." button and edit the "Path" variable.
If this variable does not contain any path containing "Qt", you must add Qt libraries path.
It should be something like <Qt install dir>/5.5/msvc2013_64/bin. This path can vary depending on the installed Qt version, check the exact path before.
If the variable editor looks like a long, semi-colon separated line, add a semi-colon followed by the path at the end of the line.
If the editor looks like a list, just add a new value to that list.

When done, you can run the executable.
Latest versions of Windows are a bit strict on security, so this is possible you obtain a security warning at opening.
Indeed, StateS is a very small application, not very well-known. Windows automatically blocks this kind of programs when downloaded from the Internet.
I guarantee there is no virus in it, so to avoid the warning, right-click on the executable, choose "Properties", check "Unlock", and hit "OK".

You can now run the application (finally!).


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
Qt 5.4 is needed, as well as a compiler supporting c++ 2011.

Most art used in the software is, or is based on, third-party art work released under open source licences.
Authors information and corresponding licences can be found in "art" folder.
