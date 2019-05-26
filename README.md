# Proj3dBook

![Screen Shot](images/startPage.png)
3dBook Update 		May 30 2018

Nearly six months have passed since updating this repo.
This time was spent getting the code cleaned up a bit.

I'm not sure how well I succeeded.  I can say: The code is here and it is much cleaner 
than it was at the end of last year. It builds and runs in my environment, which is 
VS 2017 on Windows 10. It also builds and runs on my older laptop VS 2012 on Windows 8. 
I hope it will do the same for anyone cloning it.  See doc\ building.txt. 

Along with the source code( Win32 C++ ), there are some "books" for it to read. 
What good is a reader without something to read?  Why the quotes around books? 
Well, the longest title "Rendering the Platonic Solids" is some 30 pages, and the 
rest are just a few pages.  So maybe short essays or some such.  However if a picture
is worth a thousand words, then perhaps a three dimensional rendering is worth ten thousand.  And a thirty page offering is not such a trifle.

These 3dBook-Reader "books" are Unicode files with the .tdr extension.  They are
lightly formatted so the 3dBook-Reader can respond to the text, and links in the text,
to interact with the reader.

What the heck is the 3dBook-Reader anyway?  Good question!  A Windows program with a 
"text" window and an Open GL window.  So you can have a book with full 3d graphics.  
You really need to build it and use it to know. Ex:  You could have the text to a game
in one window and the game running in the other. 

Why the quotes around text?  This "text" window is really the  directWrite - direct2D
window.  So it has all the features of those two powerful API's.  One a typesetting
engine and one a 2D graphics API.   

So developing the 3dBook-Reader means developing the books it reads and graphics it
presents along with it.  And yes: It is as much fun as it sounds!  You may feel like
Magister Ludi Joseph Knecht in Hermann Hesse's "The Glass Bead Game".

"Rendering the Platonic Solids" doesn't just write about and present code for rendering
the solids: It actually renders them in the graphics window step by step - interacting with the text and reader.

Interaction:That really is the development emphasis and challenge.


As promised:

The octahedron and dodecahedron Platonic solids have been added to complete the library
on the five Platonic solids.  If you can get the 3dBook-Reader source to compile and run;
all the Platonic solids are rendered, step by step, in "Rendering the Platonic Solids".
This file is in \doc\platonic.tdr. Or you can just read the file itself; it's a Unicode 
file, formatted for the 3dBook-Reader, but readable by a human.

## The Program. 
If you are able to get this source code to compile and run; your machine's display should resemble the screen shot above.

You see a Windows desktop parent window containing two child windows. The window on the left is the Direct Write - Direct 2D window (DW window), the window on the right is the OpenGL window (GL window).  

## The Concept:  
A prototype for a book format reader combining high quality text typesetting and realtime 3d graphics interactively.

## The 3dBook-Reader Files: 
This project is called *"The 3dBook-Reader"* ; so what does the reader read?

The **3dBook-Reader**  parses a formatted unicode text file with the .tdr file extension.  The format is similar to that of an .rtf file but simpler.  The human readability and writeabilty is on par with that of a markdown language file like this README.md.  The typesetting capabilities are similar; with sizes, bolds, italics, stike-troughs, underlines and such. Since the typesetting is implemented with the Dwrite API all its methods are available for future development.

The project is called *"The 3dBook-Reader"* , not the *"The 3dScroll-Reader"* ; so it is a **book**  and is paginated like a book.

On start up, the reader loads a file, *start.tdr*, which presents some information about the user interface:  Menus, how to page thru the book, load a file etc.

In this version the DW window controls what openGL graphics routine is run in the GL window.  The .tdr file tells the DW window which openGL routine goes with which pages.
