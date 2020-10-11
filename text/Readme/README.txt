
*******************
* StateS v. 0.4.2 *
*******************

StateS is intended to be a simple yet effective Finite State Machines (FSMs) drawer.

Currently, StateS allows you to:

- Draw a FSM,
- Simulate and verify it,
- Export it to VHDL (very experimental),
- Export it to PDF and image formats.

You can test interacting with a machine by loading file "Demo.SfsmS".

Linux version requires Qt to run.


*** How to use ***


** Windows **

Download and unzip the archive or download the executable, then launch StateS_**.exe.

Latest versions of Windows are a bit strict on security, so you may obtain a security warning at opening.
Indeed, StateS is a very small application, not very well-known. Windows automatically blocks this kind of programs when downloaded from the Internet.
The warning window is built to make difficult running the executable: the execution option is hidden.
I guarantee there is no malicious code in StateS, so to avoid the warning, right-click on the executable, choose "Properties", check "Unlock", and hit "OK".

I distribute the Windows version as a big, standalone, executable as I use an Msys2 Qt environnement which is non trivial to install for end user.
If you wish for a light, dynamic, version, it means you probably already have Qt installed to run it, thus building StateS from the git code as depicted below should be a straightforward move.


** Linux **

Install Qt from your distribution repositories using your package manager, or update to the latest version if already installed. At least Qt 5.9 is required.

Make sure the StateS file is executable:
From command line, cd to the folder containing the executable, and type "chmod +x ./<name of the file>".
Or graphically, right-click on the file, hit "Properties", and search for an "Allow execution" or similar checkbox, and check it.

Run the application.


>>> StateS is still in beta phase. <<<

If you think you found a bug in StateS, please report it at address https://sourceforge.net/p/states/tickets/
But please check known_bugs.txt file first.

Any suggestion concerning StateS can be done on the forum at address https://sourceforge.net/p/states/discussion/
Please only suggest improvments for implemented features while I haven't implemented all the base features yet.

And remember StateS in still under heavy development!
It is not intended to be complete, or even useful for now!


*** Obtaining code and generating StateS ***

StateS code license (GNU GPL V2) can be consulted using file LICENSE.txt, or at https://www.gnu.org/licenses/
To obtain code, use command:
git clone git://git.code.sf.net/p/states/code states-code
To compile, use commands `qmake` on .pro file, then `make`
Qt 5 is needed, as well as a compiler supporting c++ 2011.

* Visual Studio issues *
Under Windows, I recommand using MinGW-based toolchain as Visual Studio has non-standard exception function signature
If you still want to use VS, you'll have to edit the source to make the function signature match before compilation.


** Art **

Most art used in the software is, or is based on, third-party art work released under open source licences.
Authors information and corresponding licences can be found in the "art" folder in the git repository.
