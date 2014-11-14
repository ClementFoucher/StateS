
*****************
* StateS v. 0.1 *
*****************

This software is intended to be a simple and effective discrete event machines drawer.

I currently focus on Finite State Machines (FSMs) in order to build the platform bases, but I plan to extend the software in supporting Grafcet, and perhaps someday Petri nets.

What the software can do for now:

- Draw a FSM,
- Simulate it,
- Export to PDF.

Qt 5.3 is required to execute this application.

You can test interacting with a machine by loading file "Demo.SfsmS".

This is early stage of the software development.
Currently, there are NO UNDO when editing machine.

Here is what I plan to support (this list is only thoughts, items will maybe, or maybe not, actually implemented):
- Support bit vectors and variable affectation,
- Check machine for correctness,
- Ability to save/load timeline, and load stimuli vector,
- Add machine display customization in order to make it look as you're used to.

And, longer term (even maybe never), but I would love to implement these:
- Support a coupling interface to send outputs/receive inputs from another software (or more generally, another environment -- hardware?),
- Export machines using standard save formats (do standard representations exist?),
- Generate VHDL code from machine.

This list reflects what I have in mind currently about the software I want to build.
But some of these features may be complex to implement, and I can't guarantee I'll actually do.

What I want most is to keep StateS simple to use on any platform.
And most of all, I do not want to add UI too much complexity.


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
Qt5.3 is needed, as well as a compiler supporting -std=c++11 flag.

Most art used in the software is, or is based on, third-party art work released under open source licences.
Authors information and corresponding licences can be found in "art" folder.
