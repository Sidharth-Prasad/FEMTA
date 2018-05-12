# Welcome to the FEMTA's program documentation
## Documentation Scheme
Documentation in this file-tree is organized into many parts. In each directory is an associated README.md file explaining it's contents. Program files written by Noah Franks are documented using in-file code comments. All C code conforms to the GNU 1989/1990 standard, which is the default used by gcc, the GNU Compiler Collection.

## Notes about the Raspberry Pi 0 W
The Raspberry Pi 0 W, which shall be refered to as the 'pi', has certain characteristics that make programming for it different than for alternative models. Please note that the pi has only a single-core ARM CPU, meaning concurrency models are more lax. State information need not be shared between cores, since only one core exists. This means that if this code is ran on a differing model such as the Raspberry Pi 3, undeterminism may become a major issue. Although context switching has been considered when developing this software suite, it is likely that the introduction of multiple cores may pose serious race issues.  

## Raspian idosyncracies
Raspian Lite, the operating system installed on the pi, has quirks that need to be considered when developing software.  
- Serial UART. By default, the serial interface will attempt to write a terminal upon connection. This must be turned off. Check raspian's /boot/config.txt and /etc/* for more information.

## Code style
Although all programs conform to the GNU 1989/1990 standard, Noah (who is writing this) has not used any style guide. Please do not change this, as we do not expect this code to be used by a large team of developers, meaning that time spent lenting will be wasted. If you would like to know Noah's full opinion on the matter, please execute physical/jolly-roger.sh for more information.  

In all seriousness, code should be aesthetic. Remember, "If you don't have time to do it right the first time, how will you have time to do it right the second?" Although this is hard to achieve and I definately need to improve, I really do think that, for the context of this software, writing such that it's easy to read and understand is more important than using a 'standard' number of spaces or banning C style pointer casting. Also it's the 21'st centery, your editor will deal with tabs vs spaces.  

## Emacs
If you aren't using Emacs, please reconsider, for it is one of the most beautiful programs ever written and serves as an all purpose terminal text editor. The following formats need or are greatly improved by the editor.

-.emacs, the configuration format. This is typically located at ~/.emacs, if it exists.
-.org, the organization format. Requires org mode.
-.md, the markdown format. Install the following packege via .emacs and the meta command line.
![](markdown_elisp_white.png)


## With great power comes great responsability
Upon access to the cubesat's code and association with the sudoers group, you will have full permissions to do everything from shooting yourself in the foot to malicious activities. As is intuitive, no system is fully secure, and it's often the people you trust rather than the systems in place that makes the difference. [Reflections on Trusting Trust][1]. Just because you *can* do something doesn't mean that you *should*. Please do not copy the contents of the pi's SD card for personal use. Likewise please keep track of where the pi is at all times. Sensitive information is present, from the mail server to my PAL connection, so please be responsible. Tony had his Pi hacked while on the university network; malicious individuals do exist. Whatever you do, DO NOT PORT FORWARD THE PI UNLESS YOU KNOW EXACTLY WHAT YOU ARE DOING. 

[1]:https://www.ece.cmu.edu/~ganger/712.fall02/papers/p761-thompson.pdf "Reflections on Trusting Trust"


## The subdirectories are as follows
### [meetings][2]
Contains a meeting log of the .org format, which is an Emacs format.
### [physical][3]
Contains hardware tests, pinout information, sample python BNO code, and Steven Peguia's C# serial code (which Noah has ported to C).

### [scratch][4]
Contains scratch work for testing short pieces of code. Some files are outdated excepts from the spacebound directory's contents.

### [spacebound][5]
Contains the main project. Files are usually the most recent versions of each program.

### [virtual][6]
Contains all test scripts for terminal interfacing. Execute virtual/256-color-test.sh to check color compatibility.

[2]:https://github.com/Noah-Franks/FEMTA/tree/master/meetings   "Go to directory"
[3]:https://github.com/Noah-Franks/FEMTA/tree/master/physical   "Go to directory"
[4]:https://github.com/Noah-Franks/FEMTA/tree/master/scratch    "Go to directory"
[5]:https://github.com/Noah-Franks/FEMTA/tree/master/spacebound "Go to directory"
[6]:https://github.com/Noah-Franks/FEMTA/tree/master/virtual    "Go to directory"
