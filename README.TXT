Welcome to You Only Live Once!

_**Note:** I'm not the author of this rogue-like. It was created by Jeff Lait as a 7drl in 2005-2006, all credits go to him.
This repository was set up merely to preserve a historical artifact and to provide means to build/run it on newer systems.
Original web page with all the releases can be found [here](http://www.zincland.com/7drl/liveonce/).
Original binary releases can be found in the [list of releases](https://github.com/clckwrkbdgr/liveonce/releases).
_

## From original web page:

The features of You Only Live Once are:

* Extremely tactical combat. There is no randomness in combat. You always hit and always do full damage. This means that careful placement is the difference between success and failure.
* Heavy on plot and characterization. A story is told. It is not just kill things for no reason. (If you want to just kill things for no reason, [POWDER](http://www.zincland.com/powder) is recommended)
* Quick to complete. You can likely fully explore and enjoy the game in an hour or two. You are then free to get on with your life. 

Have fun! 


===========================
  ABOUT
===========================
`
This is a Seven Day Roguelike.  It was written in a 168 hour time
frame.  Hopefully it doesn't feel like that was the case, however.

You Only Live Once is written by Jeff Lait and uses either SDL or curses
for graphical output.

You can contact Jeff Lait at: jmlait [snail] zincland [dot] com.

===========================
  HOW TO RUN
===========================

If you are a windows users, run it from windows/liveonce.exe.
If you want the windows curses version, make sure you have 30 rows
in your console window and run from:
windows_curses/liveonce.exe.

If you are a linux user, try:
cd linux
./liveonce

If you want a linux console version, make sure you have 30 rows in
your console window and run from:
cd linux_curses
./liveonce

If it complains about lack of permissions, I apologize for poor
permission management and do:
chmod +x liveonce

If you are a mac user, the file you want is in the mac subdirectory 
called liveonce.  I've only used the command line to run this
remotely, so I won't guarantee success.

If you are a source diver, go into the src directory and know much
fear.  Look for BUILD.TXT files to try and guide you.

===========================
  HOW TO PLAY
===========================

Movement is done with the number pad or with vi style keys.  You 
can also use the cursor keys for four way movement.
    7 8 9  y k u
     \|/    \|/
    4-5-6  h-.-l
     /|\    /|\
    1 2 3  b j n

To chat with someone, bump into them.  To attack someone in melee,
bump into them.  
 Quick Keys:
   a - About LiveOnce
   c - Character sheet
   f - Fire ranged weapon
   Q - Quit and Save
   w - Welcome message
   x - Look around
   ? - This help
   >, < - Climb staircase

===========================
  CUSTOMIZING FONT
===========================

If you don't like the default font, you can change it!

Go to your executable directory and note the file alphabet.bmp.  Go to
src/gfx/alphabet.txt and display that text.  Take a screen capture,
crop the relevant area, and save as a .bmp.  Overwrite your executable
directory's version of alphabet.bmp, and the new font should be
running!

===========================
  FINAL IMPORTANT NOTE
===========================

Have fun!
