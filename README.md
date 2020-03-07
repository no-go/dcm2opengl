# dcm2opengl

This code collects gray value images (MRT scans), join pixels of special gray values (skin or bones) with opencv
and generates a openGL 3D model (Window).

the code is very old (2013 ?) with old opengl and massive simplified.

Parameter in `main.cpp` code (not command line!):

- FOLDER: path to the images folder
- SIZE: The x-y-size of the dcm file in pixel
- START and STOP: the values for the image number in the files (vhf.IMAGE_NUMBER.dcm)
- JUMP: e.g. with a 10 at this place, only every 10th image and pixel is used for the result (faster!)
- EXTRACTVALUE: use 600 or 700 for the skin, or use 1300 for the bones

## Usage: click into window!

Initial the Windows is gray and empty.

- `space`: add a layer to 3D
- `+`: zoom in
- `-`: zoom out
- `c`: add a point to the layer

## make

You need GLUT (tested with freeglut 3.2.1), glu (tested with 9.0.1) and openvc (tested with 4.2.0).
Additionaly `pkg-config` is a good choice. GL is also used (tested mesa 19.3.4).

## example files

https://imaging.nci.nih.gov/nbia-search/

