# Image-Filtering
Program implements filtering across ppm images to output a blurred result of the input image. 

    This program applies a 7 x 7 mean filter in 3 implementations:
       - Basic 2D Convolution
       - Seperable Filters 
       - Sliding Window 
    
    Each filter is applied ten times and the average run time is 
    printed out to screen. The purpose of this project is for 
    showing runtime differences in different loop complexities when
    filtering ppm images - sliding window being the best (fastest).

 
