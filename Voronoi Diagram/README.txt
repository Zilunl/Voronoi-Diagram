This is an incomplete base code for assignment 2.
You can compile, and try to run stage 2

Content:
- function doSplit updated for doing split of ass2.
  Note that "struct split" is designed slightly different from that of 
  Grady's code.  
- The files from Anh Ass1 Solution, with a small change to face_t so that it can be conveniently used for Ass2
- The euclide_space module now include basic "black box" functions for finding intersection between 2 straight line segments, as well as between a straight line segment and a line. The module also contains some explanations on how to use the function intersect.
- Note that I heavily use typdedef. If you don't like typedef, you can just ignore them, and replace all appearances of "XXX_t" by "struct XXX". That should work if you replace all of them, and delete all the lines typedef.

Notes:
- The base code is not in the same level as what provided by Grady. Namely, you can not rely on searching for "fill in" and do the filling in.
- The main points I'd like you to look at are:
     + euclid_space module, to understand intersects, and to plug-in and use in your code
	 + function doSplit which is similar to Grady's applySplit and
       can be used for performing split of ass2
     + voronoi2.c with a structure which is similar to the ones you wrote in your previous C courses
- The other modules are basically copied from my Ass1 solution.

Good luck!
