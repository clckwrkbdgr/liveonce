Welcome to You Only Live Once!

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
