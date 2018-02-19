//Create Test Image of random numbers
//
//To Compile: gcc RandTestImg.c -lpng -o RandTestImg
//
//-lpng inlcudes png library


#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h> 

// Color Pixel

typedef struct {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
} pixel_t; 

// Picture

typedef struct {
        pixel_t *pixels;
        size_t width;
        size_t height;
} bitmap_t; 

//Given "bitmap" returns pixel at the point x,y

static pixel_t * pixel_at ( bitmap_t * bitmap, int x, int y){
        return bitmap->pixels + bitmap->width * y + x;
}

//Write 'bitmap'  to png file specified by path 
//return 0 sucess, else error

static int save_png_to_file (bitmap_t *bitmap, const char *path){
        FILE * fp;
        png_structp png_ptr = NULL;
        png_infop info_ptr = NULL; 
        size_t x, y;
        png_byte ** row_pointers = NULL;

        //default status is failure
        int status = -1; 

        int pixel_size = 3; //trial and error
        int depth = 8;

        fp = fopen (path, "wb");
        if (! fp){
                goto fopen_failed;
        }

        png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (png_ptr == NULL){
                goto png_create_write_struct_failed;
        }

        info_ptr = png_create_info_struct (png_ptr);
        if (info_ptr == NULL){
                goto png_create_info_struct_failed;
        }

        //Error handling
        if (setjmp (png_jmpbuf (png_ptr))){
                goto png_failure;
        }

        //Set image attributes
        png_set_IHDR (  png_ptr,
                        info_ptr,
                        bitmap->width,
                        bitmap->height,
                        depth,
                        PNG_COLOR_TYPE_RGB,
                        PNG_INTERLACE_NONE,
                        PNG_COMPRESSION_TYPE_DEFAULT,
                        PNG_FILTER_TYPE_DEFAULT);

        //inital rows of PNG
        row_pointers = png_malloc (png_ptr, bitmap->height * sizeof(png_byte *));
        for (y = 0; y < bitmap->height; ++y){
                png_byte  * row = png_malloc( png_ptr, sizeof(uint8_t) * bitmap->width * pixel_size);
                row_pointers[y] = row;
                for ( x = 0; x < bitmap->width; ++x){
                        pixel_t * pixel = pixel_at (bitmap, x, y);
                        *row++ = pixel->red;
                        *row++ = pixel->green;
                        *row++ = pixel->blue;
                }
        }
        
        //Write image data

        png_init_io (png_ptr, fp);
        png_set_rows (png_ptr, info_ptr, row_pointers);
        png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

        //Success
        status = 0;

        for (y = 0; y < bitmap->height; y++){
                png_free (png_ptr, row_pointers[y]);
        }
        png_free (png_ptr, row_pointers);

png_failure:
png_create_info_struct_failed:
        png_destroy_write_struct (&png_ptr, &info_ptr);
png_create_write_struct_failed:
        fclose (fp);
fopen_failed:
        return status;
}


static int pix (int value, int max){
        if (value < 0)
                return 0;
        return (int) (256.0 * ((double) (value)/(double) max));
}

static int adjust_to_pix_value (uint64_t num_value){
        return (int) 256 * ( (double) num_value / (double) UINT64_MAX ); 
}


uint64_t xorshift128plus(uint64_t s[2]){
        uint64_t x = s[0];
        uint64_t const y = s[1];
        s[0] = y; 
        x ^= x << 23; 
        s[1] = x ^ y ^ (x >> 17) ^ (y >> 26);
        return s[1] + y; 
}


int main(){
       

        uint64_t s[2] = {1,1}; 

        time_t t;
        srand((unsigned) time(&t));


        s[0] = rand(); 
        s[1] = rand();

        //First 10 are not so good
        uint64_t waste;
        int k;
        for(k = 0; k<10; k++){
                waste  = xorshift128plus(s);
                printf (" %" PRIu64 "\n", waste); 
                printf (" %d\n", adjust_to_pix_value(waste)); 
        

        }


        bitmap_t fruit;
        int x;
        int y;

        //create image
        fruit.width = 800;
        fruit.height = 800;

        fruit.pixels = calloc (sizeof (pixel_t), fruit.width * fruit.height);


        int color = 256;

        for ( y=0; y < fruit.height; y++){
                for ( x=0; x< fruit.width; x++){
                        pixel_t * pixel = pixel_at (& fruit, x, y);
                        
                        color = adjust_to_pix_value(xorshift128plus(s));

                        //printf("%d ", color);

                        pixel->red = color;
                        //pixel->red = pix(x, fruit.width);
                        pixel->green = color;
                        //pixel->green = pix(y, fruit.height);
                        pixel->blue = color;
                }
        }
        
        //write image to file

        save_png_to_file (& fruit, "fruit.png");

        return 0;
}

