/*  
 *  John Lawler - ECE 4310 - Lab 1 - filtering ppm images

    This program applies a 7 x 7 mean filter in 3 implementations:
       - Basic 2D Convolution
       - Seperable Filters 
       - Sliding Window 
    
    FL is used to indicate the filter length of one side of the desired
    filter array. FL = 3 means that we are using a 7 x 7 array, as the 
    epicenter can reach 3 tiles left or right, up or down. 

    Each filter is applied ten times and the average run time is 
    printed out to screen. The purpose of this project is for 
    showing runtime differences in different loop complexities when
    filtering ppm images - sliding window being the best (fastest).

 ** To compile, must link using -lrt  (man clock_gettime() function). 

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define STARTCOL 3
#define FL 3
#define FILTERSIZE 7
#define FILTERAREA FILTERSIZE * FILTERSIZE

/* Function to query time */
struct timespec get_time( struct timespec tp) {
    clock_gettime(CLOCK_REALTIME,&tp);
   // printf("%ld %ld\n",(long int)tp.tv_sec,tp.tv_nsec);
    return tp;
}

/* Function used to find the difference between runtimes */
long int printDifference(struct timespec tp1, struct timespec tp2) {
    long int runtime;

    // Find time difference to pixelVal later for finding average runtime 
    runtime = tp2.tv_nsec - tp1.tv_nsec;
    if (tp1.tv_nsec > tp2.tv_nsec) runtime = runtime + 1e9;

    printf("%ld\n", runtime);
    return runtime;

}

/* Function to handle printing results to output file */
void printOutput (char *filename, int ColCount, int RowCount, unsigned char *output) {
    FILE *fpt;
    // Write output to a file
    fpt = fopen(filename, "w");
    fprintf(fpt,"P5 %d %d 255\n", ColCount, RowCount);
    fwrite(output, ColCount*RowCount, 1, fpt);
    fclose(fpt);
}

int main(){
    struct timespec	tp1,tp2;
    unsigned char	*image;
    unsigned char	*output;
    long int        timesum;
    char		    header[320];
    int		        RowCount, ColCount, BYTES, *o1;
    int		        r, c, r2, c2, i, j;
    int             pixelVal;
    FILE		    *fpt;
    
    // Open ppm image for handling
    if ((fpt = fopen("bridge.ppm", "rb")) == NULL) {
      printf("Unable to open bridge.ppm for reading\n");
      exit(0);
      }
    
    // Read ppm header into vals
    fscanf(fpt, "%s %d %d %d", header, &ColCount, &RowCount, &BYTES);
    if ( strcmp(header, "P5") != 0  ||  BYTES != 255) {
      printf("Not a greyscale 8-bit PPM image\n");
      exit(0);
      }
    
    image = (unsigned char *)calloc(RowCount*ColCount, sizeof(unsigned char));
    header[0] = fgetc(fpt);	/* read white-space character that separates header */
    fread(image, 1, ColCount*RowCount, fpt);
    fclose(fpt);
    
    // Loop between filter implementations
    for (i = 0; i < 3; i++ ) {

        // Nullify for each filter
        timesum = 0;
        pixelVal = 0;
        output=(unsigned char *)calloc(RowCount*ColCount, sizeof(unsigned char));
        o1=(int *)calloc(RowCount*ColCount,sizeof(int));

        // Run each filter 10 times
        for (j = 0; j < 10; j++) {
            switch(i) {
                case 0:     /* 2D Convolution   */
                    // Get time prior
                    tp1 = get_time(tp1);

                    // Loop, beginning at inner square to avoid edge cases
                    for (r = FL; r < RowCount-FL; r++) {
                        for (c = FL; c < ColCount-FL; c++) {
                            pixelVal = 0;
                            // Inside loop for handling the 7x7 around indexed pixel to convolve
                            for (r2 = -FL; r2 <= FL; r2++) {
                                for (c2 = -FL; c2 <= FL; c2++)
                                    pixelVal += image[ColCount*(r+r2) + (c+c2)];
                            }
                        output[ColCount*r + c] = pixelVal/FILTERAREA;
                        }
                    }

                    // Get time after execution
                    tp2 = get_time(tp2);
                    timesum += printDifference(tp1, tp2);
                    if (j == 9){
                        printf("\n 2D Convolution - average runtime: %ld \n\n", timesum/10);
                        printOutput("2D-Filter.ppm", ColCount, RowCount, output);
                    }
                    break;
                case 1:     /* Seperable Filters */
                    tp1 = get_time(tp1);
                    for (r = FL; r < RowCount-FL; r++) {
                        for (c = 0; c < ColCount; c++) {
                            pixelVal = 0;
                            // Inside loop to just find convolution of each 7x1 col
                            for (r2 = -FL; r2 <= FL; r2++)
                                pixelVal += image[ColCount*(r+r2) + c];
                            o1[ColCount*r + c] = pixelVal;
                        }
                    }
                    // Convolve for actual output from the 7x1 to 7x7
                    for (r = FL; r < RowCount-FL; r++) {
                        for (c = FL; c < ColCount-FL; c++) {
                            pixelVal = 0;
                            // Convolution for the 1 x 7 across the cols each row
                            for (c2 = -FL; c2 <= FL; c2++)
                                pixelVal += o1[ColCount*r + (c+c2)];
                            output[ColCount*r + c] = pixelVal/FILTERAREA;
                        }
                    }
                    // Get time after execution
                    tp2 = get_time(tp2);
                    timesum += printDifference(tp1, tp2);
                    if (j == 9){
                        printf("\n Seperable Filters - average runtime: %ld \n\n", timesum/10);
                        printOutput("seperable-Filter.ppm", ColCount, RowCount, output);
                    }
                    break;
                case 2:     /* Sliding Window   */
                    tp1 = get_time(tp1);
                    for (r = FL; r < RowCount-FL; r++) {
                        for (c = 0; c < ColCount; c++) {
                            pixelVal = 0;
                            // nested for-loops as sep filters
                            for (r2 = -FL; r2 <= FL; r2++)
                                pixelVal += image[ColCount*(r+r2) + c];
                            o1[ColCount*r + c] = pixelVal;
                        }
                    }
                    // Convolve for actual output
                    for (r = FL; r < RowCount-FL; r++) 
                        for (c = FL; c < ColCount-FL; c++) {
                            // Check if the ppm index = beginning of new row to start
                            // intitial convolution
                            if (c == STARTCOL) {
                                pixelVal = 0;
                                // Perform regular sep filter summing for start of each row in ppm 
                                for (c2 = -FL; c2 <= FL; c2++)
                                    pixelVal += o1[ColCount*r + (c+c2)];
                                output[ColCount*r + c] = pixelVal/FILTERAREA;
                            /* if the window is past the initial column
                             * then add/subtract the front and back */
                            } else {
                                // Find pixelVal sum from the already convolved values
                                pixelVal = pixelVal - o1[ColCount*r + c - FL - 1] + o1[ColCount*r + c + FL];
                                output[ColCount*r + c] = pixelVal/FILTERAREA;
                            }
                        }
                    // Retrieve time for sliding window
                    tp2 = get_time(tp2);
                    timesum += printDifference(tp1, tp2);
                    if (j == 9){
                        printf("\n Sliding Window Filters - average runtime: %ld \n\n", timesum/10);
                        printOutput("slide-Filter.ppm", ColCount, RowCount, output);
                    }

                    break;
            } /* end of switch */
        } /* end of  for -> 10 */
    }
}

