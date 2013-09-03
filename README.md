colors_in_image
===============
Currently being rewritten using the opencv image library.

An attempt to detect the dominate, most important colors in an image.  The algorithm goes through each pixel, saving its rgb value with a weight calculated based on the variance of surrounding pixels of a grayscale, edge-detected convolution of the image (horizontal and vertical edges are removed if using a 2d kernel).  The extracted rgb values are then grouped together by their closest matches to defined colors (name, hex, rgb value), and sorted by colors with pixels in the most turbulent areas of the image. 

## Examples

  ![](https://raw.github.com/selbyk/colors_in_image/master/images/example.jpg)
