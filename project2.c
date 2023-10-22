#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

// Struct for an image, containing its dimensions and pixel data
struct bitmap
{
	int width;
	int height;
	int *pixels;
};

const int DIB_HEADER_SIZE = 14;
const int BMP_HEADER_SIZE = 40;

// Make "byte" mean "unsigned char"
typedef unsigned char byte;

// Calculates the stride of a .bmp file.
// (The stride is how many bytes of memory a single row of
// the image requires.)
inline int bmp_file_stride(struct bitmap *bmp);

// Calculates the total size that a .bmp file for this bitmap
// would need (in bytes)
inline int bmp_file_size(struct bitmap *bmp);

// Opens the file with the given name and maps it into memory
// so that we can access its contents through pointers.
void *map_file_for_reading(char *filename);

// Opens (and creates if necessary) the file with the given name
// and maps it into memory so that we can access its contents
// through pointers. 
void *map_file_for_writing(char *filename, int file_size);

// Takes the contents of a bitmap file (bmp_file) and reads
// its data, filling in the struct bitmap pointed to by bmp.
// Returns 0 if everything worked, -1 if the file data isn't
// valid.
int read_bitmap(void *bmp_file, struct bitmap *bmp);

void write_bitmap(void *bmp_file, struct bitmap *bmp);


// Converts between a packed pixel (0xRRGGBB) and its components.
void rgb_to_pixel(int *p, int r, int g, int b);
void pixel_to_rgb(int p, int *r, int *g, int *b);

//Grayscale
void bitmap_to_grayscale(struct bitmap *bmp);

//Posterizing
void bitmap_posterize(struct bitmap *bmp);

//Mirroring
void bitmap_mirror(struct bitmap *bmp);

//Squashing
void bitmap_squash(struct bitmap *bmp);

//Reflecting
void bitmap_reflect(struct bitmap *bmp);

//Rotating
void bitmap_rotate(struct bitmap *bmp);

//Skewing
void bitmap_skew(struct bitmap *bmp);

//Shrinking
void bitmap_shrink(struct bitmap *bmp);

/* Please note: if your program has a main() function, then
 * the test programs given to you will not run (your main()
 * will override the test program's). When running a test,
 * make sure to either comment out or rename your main()
 * function!
 */

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        printf("There is no image specified in the command line\n");
        return 1;
    }
    else if (argc == 2)
    {
        char *filename = argv[1];
        char input[20];

        int *pointer = map_file_for_reading(filename);

        struct bitmap t_bmp;
        read_bitmap(pointer, &t_bmp);
        long file_size = bmp_file_size(&t_bmp);
        munmap(pointer, file_size);

        while (input[0] != 'q')
        {
            printf("Menu:\n");
            printf("\tG) Make grayscale\n");
            printf("\tP) Posterize\n");
            printf("\tU) Squash\n");
            printf("\tM) Mirror\n");
            printf("\tR) Reflect\n");
            printf("\tO) Rotate\n");
            printf("\tK) Skew\n");
            printf("\tH) Shrink\n");
            printf("\tS) Save\n");
            printf("\tQ) Quit\n");

            printf("What would you like to do? ");
            scanf("%s", input);

            if (input[0] == 'g')
            {
                bitmap_to_grayscale(&t_bmp);
                printf("\nGrayscale selected\n");

            }
            else if (input[0] == 'p')
            {
                bitmap_posterize(&t_bmp);
                printf("\nPosterized selected\n");
            }
            else if (input[0] == 'u')
            {
                printf("\nSquash selected\n");
                bitmap_squash(&t_bmp);
            }
            else if (input[0] == 'm')
            {
                printf("\nMirror selected\n");
                bitmap_mirror(&t_bmp);
            }
            else if (input[0] == 'r')
            {
                printf("\nReflect selected\n");
                bitmap_reflect(&t_bmp);
            }
            else if (input[0] == 'o')
            {
                printf("\nRotate selected\n");
                bitmap_rotate(&t_bmp);
            }
            else if (input[0] == 'k')
            {
                printf("\nSkew selected\n");
                bitmap_skew(&t_bmp);
            }
            else if (input[0] == 'h')
            {
                printf("\nShrink selected\n");
                bitmap_shrink(&t_bmp);
            }
            else if (input[0] == 's')
            {
                printf("\nEnter filename: ");
                scanf("%s", input);
                char *o_filename = input;
                printf("\nSaving to %s", input);
                int file_size_updated = bmp_file_size(&t_bmp);
                int *o_pointer = map_file_for_writing(o_filename, file_size_updated);
                write_bitmap(o_pointer, &t_bmp);
                printf("\nSaved!\n");
                long length = file_size_updated;
                munmap(o_pointer, length);
            }
            else if (input[0] == 'q')
            {
                printf("\nBye.\n");
                return 0;
            }
        }
    }


    //*/ My code I used for testing

    char *filename = "/var/project02/images/converse.bmp";
    int *pointer = map_file_for_reading(filename);

    struct bitmap o_bmp;
    read_bitmap(pointer, &o_bmp);
    long file_size = bmp_file_size(&o_bmp);

    //bitmap_to_grayscale(&o_bmp);

    //bitmap_posterize(&o_bmp);

    //bitmap_mirror(&o_bmp);

    //bitmap_squash(&o_bmp);

    //bitmap_reflect(&o_bmp);

    //bitmap_rotate(&o_bmp);

    bitmap_skew(&o_bmp);

    //bitmap_shrink(&o_bmp);

    /*int file_size_updated = bmp_file_size(&o_bmp);
    
    char *o_filename = "test_skew.bmp";
    int munmap(pointer, file_size);
    int *o_pointer = map_file_for_writing(o_filename, file_size_updated);

    write_bitmap(o_pointer, &o_bmp);
    long length = file_size_updated;
    int munmap(o_pointer, length);
    //*/
    
	return 0;
}
// */

int bmp_file_stride(struct bitmap *bmp)
{
    return (24 * bmp->width + 31) / 32 * 4;
}

int bmp_file_size(struct bitmap *bmp)
{
    int stride = bmp_file_stride(bmp);
    return DIB_HEADER_SIZE
        + BMP_HEADER_SIZE
        + stride * bmp->height;
}

void *map_file_for_reading(char *filename)
{
    
    struct stat statbuf;
    
    // A) Use open() to open the file for reading.

    int fd = open(filename, O_RDONLY);
    if (fd == -1) 
    {
        perror(NULL);
        return NULL;
    }
    
    // B) Use fstat() to determine the size of the file.
    fstat(fd, &statbuf);

    if (fstat(fd, &statbuf) == -1)
    {
        perror(NULL);
        return NULL;
    }
    long size = statbuf.st_size;
    
    // C) Call mmap() to map the file into memory.
    int *p = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);

    if (p == MAP_FAILED)
    {
        perror(NULL);
        return NULL;
    }

    // D) Close the file using close().
    close(fd);

    // E) Return the pointer returned from mmap().
    return p;

    // Default implementation: just returns NULL.
    //return NULL;
}

void *map_file_for_writing(char *filename, int file_size)
{

    // A) Use open() to open the file for writing.
    int fd = open(filename, O_RDWR | O_CREAT, 0644);
    if (fd == -1) 
    {
        perror(NULL);
        return NULL;
    }
    
    // B) Use ftruncate() to set the size of the file.
    int ftru = ftruncate(fd, file_size);
    if (ftru == -1)
    {
        perror(NULL);
        return (NULL);
    }

    // C) Call mmap() to map the file into memory.
    int *p = mmap(NULL, file_size, PROT_WRITE, MAP_SHARED, fd, 0);

    if (p == MAP_FAILED)
    {
        perror(NULL);
        return NULL;
    }

    // D) Close the file using close().
    close(fd);

    // E) Return the pointer returned from mmap().
    return p;

    // Default implementation: just returns NULL.
    //return NULL;

}

int read_bitmap(void *bmp_file, struct bitmap *bmp)
{
    // Cast bmp_file to a byte * so we can access it
    // byte by byte.
    byte *file = (byte *) bmp_file;

    // Check the magic: it should start with "BM"

    if (file[0] == 'B')
    {
        if (file[1] == 'M')
        {
            bmp->width = *((int *)(file + 18));
            bmp->height = *((int *)(file + 22));
            int *pix = (int *)(file + 10);
            short *cdep = (short *)(file + 28);
            if (*cdep != 24)
            {
                printf("Color depth is not 24");
                return -1;
            }
            int *compmeth = (int *)(file + 30);
            if (*compmeth != 0)
            {
                printf("The compression method is not 0");
                return -1;
            }

            int stride = bmp_file_stride(bmp);
            //allocating memory for the pixels
            bmp->pixels = (int *) malloc(bmp->width * bmp->height * sizeof(int));
            
            for (int y = 0; y < bmp->height; ++y)
            {
                // Calculate where this row of pixels begins in the file
                // (Use the pixel data offset and stride)

                int bott_height = bmp->height - (y+1);
                
                // Remember that the file stores rows from bottom to top!
    
                for (int x = 0; x < bmp->width; ++x)
                {

                    int start = *pix + (bott_height * stride) + (3 * x);

                    // Locate the B, G, and R bytes for the pixel (x, y)
                    int b = file[start];
                    int g = file[start + 1];
                    int r = file[start + 2];
        
                    // Assemble the R, G, B into a pixel value by using rgb_to_pixel()
                    int stored_pixel;
                    rgb_to_pixel(&stored_pixel, r, g, b);
                    
        
                    // Place the pixel (x, y) at the appropriate place in the pixels array
                    int i = y * bmp->width + x;
                    bmp->pixels[i] = stored_pixel;
                }
            }
            return 0;
        }
        else
        {
            printf("Error: Not a bitmap!");
            return -1;
        }
    }
    else
    {
        printf("Error: Not a bitmap!");
        return -1;
    }

    

    // Default implementation: just returns 0.
    return 0;
}

void write_bitmap(void *bmp_file, struct bitmap *bmp)
{
    
    // Cast bmp_file to a byte * so we can access it
    // byte by byte.
    byte *file = (byte *) bmp_file;

    // Fill in the rest :)
    //Magic
    file[0] = 'B';
    file[1] = 'M';
    //File Size
    *((int *)(file +2)) = bmp_file_size(bmp);
    //Pixel Data Offset
    file[10] = 54;
    //Header Size
    file[14] = 40;
    //Bitmap Width
    *((int *)(file + 18)) = bmp->width;
    //Bitmap Height
    *((int *)(file + 22)) = bmp->height;
    //Color Panes
    file[26] = 1;
    //Color Depth
    file[28] = 24;
    //Compression Method
    file[30] = 0;
    //Image Size
    int stride = bmp_file_stride(bmp);
    *((int *)(file +34)) = bmp->height * stride;
    //Palette Size
    file[46] = 0;

    //Pixel Data
    //bmp->pixels = (int *) malloc(bmp->width * bmp->height * sizeof(int));
            
            for (int y = 0; y < bmp->height; ++y)
            {
                int bott_height = bmp->height - (y+1);
             
                for (int x = 0; x < bmp->width; ++x)
                {

                    int start = 54 + (bott_height * stride) + (3 * x);

                    int r;
                    int g;
                    int b;

                    int i = y * bmp->width + x;
                    
                    int stored_pixel = bmp->pixels[i];
                    
                    pixel_to_rgb(stored_pixel, &r, &g, &b);

                    file[start] = b;
                    file[start + 1] = g;
                    file[start + 2] = r;
                }
            }
}

void rgb_to_pixel(int *p, int r, int g, int b)
{
    // Pack r, g, and b into an int value and save
    // into what p points to
    *p = ((r & 0xff) << 16) |
        ((g & 0xff) << 8) |
        (b & 0xff);
}

void pixel_to_rgb(int p, int *r, int *g, int *b)
{
    // Separate the pixel p into its components
    // and save in the pointers
    *r = (p & 0xff0000) >> 16;
    *g = (p & 0x00ff00) >> 8;
    *b = (p & 0xff);
}

void bitmap_to_grayscale(struct bitmap *bmp)
{
    for (int y = 0; y < bmp->height; ++y)
            {
                for (int x = 0; x < bmp->width; ++x)
                {
                    int r;
                    int g;
                    int b;

                    int i = y * bmp->width + x;
                    
                    int stored_pixel = bmp->pixels[i];
                    
                    pixel_to_rgb(stored_pixel, &r, &g, &b);
                    int grayscale = (r+g+b)/3;
                    int changed_pixel;
                    rgb_to_pixel(&changed_pixel, grayscale, grayscale, grayscale);
                    bmp->pixels[i] = changed_pixel;

                }
            }
}

void bitmap_posterize(struct bitmap *bmp)
{
    for (int y = 0; y < bmp->height; ++y)
            {
                for (int x = 0; x < bmp->width; ++x)
                {
                    int r;
                    int g;
                    int b;

                    int i = y * bmp->width + x;
                    
                    int stored_pixel = bmp->pixels[i];
                    
                    pixel_to_rgb(stored_pixel, &r, &g, &b);

                    //Less than 32
                    if (r < 32)
                    {
                        r = 0;
                    }
                    else if (32 <= r && r <= 95)
                    {
                        r = 64;
                    }
                    else if (96 <= r && r <= 159)
                    {
                        r = 128;
                    }
                    else if (160 <= r && r <= 223)
                    {
                        r = 192;
                    }
                    else if (r >= 224)
                    {
                        r = 255;
                    }


                    if (g < 32)
                    {
                        g = 0;
                    }
                    else if (32 <= g && g <= 95)
                    {
                        g = 64;
                    }
                    else if (96 <= g && g <= 159)
                    {
                        g = 128;
                    }
                    else if (160 <= g && g <= 223)
                    {
                        g = 192;
                    }
                    else if (g >= 224)
                    {
                        g = 255;
                    }

                    
                    if (b < 32)
                    {
                        b = 0;
                    }
                    else if (32 <= b && b <= 95)
                    {
                        b = 64;
                    }
                    else if (96 <= b && b <= 159)
                    {
                        b = 128;
                    }
                    else if (160 <= b && b <= 223)
                    {
                        b = 192;
                    }
                    else if (b >= 255)
                    {
                        b = 255;
                    }

                    int changed_pixel;
                    rgb_to_pixel(&changed_pixel, r, g, b);
                    bmp->pixels[i] = changed_pixel;
                }
            }
}

void bitmap_mirror(struct bitmap *bmp)
{
    int new_width = bmp->width * 2;

    int *new_pixels = (int *) malloc(new_width * bmp->height * sizeof(int));


        for (int y = 0; y < bmp->height; ++y)
            {
                for (int x = 0; x < bmp->width; ++x)
                {
                    int stored_pixel = bmp->pixels[y * bmp->width + x];

                    new_pixels[y * new_width + x] = stored_pixel;
                    new_pixels[y * new_width + (new_width - x - 1)] = stored_pixel;
                }
            }
            
            free(bmp->pixels);
            bmp->pixels = new_pixels;
            bmp->width = new_width;

}


void bitmap_squash(struct bitmap *bmp)
{
	int new_width = bmp->width / 2;

    int *new_pixels = (int *) malloc(new_width * bmp->height * sizeof(int));

        for (int y = 0; y < bmp->height; ++y)
            {
                for (int x = 0; x < bmp->width;)
                {
                    //get rgb and average red green and blue seperate
                    int m = y * new_width + (x / 2);
                    int n = y * bmp->width + x;
                    int o = y * bmp->width + x + 1;

                    int first_pixel = bmp->pixels[n];
                    int second_pixel = bmp->pixels[o];

                    int r1, g1, b1, r2, g2, b2;
                    
                    pixel_to_rgb(first_pixel, &r1, &g1, &b1);
                    pixel_to_rgb(second_pixel, &r2, &g2, &b2);

                    int r = (r1 + r2) / 2;
                    int g = (g1 + g2) / 2;
                    int b = (b1 + b2) / 2;
                    int changed_pixel;
                    rgb_to_pixel(&changed_pixel, r, g, b);
                    new_pixels[m] = changed_pixel;
                    x+= 2;
                }
            }
            
            free(bmp->pixels);
            bmp->pixels = new_pixels;
            bmp->width = new_width;
}
 
void bitmap_reflect(struct bitmap *bmp)
{

    int *new_pixels = (int *) malloc(bmp->width * bmp->height * sizeof(int));


        for (int y = 0; y < bmp->height; ++y)
            {
                for (int x = 0; x < bmp->width; ++x)
                {
                    int stored_pixel = bmp->pixels[y * bmp->width + x];

                    new_pixels[y * bmp->width + (bmp->width - x + 1)] = stored_pixel;
                }
            }
            
        free(bmp->pixels);
        bmp->pixels = new_pixels;
}

void bitmap_rotate(struct bitmap *bmp)
{
    int new_width = bmp->height;
    int new_height = bmp->width;
    
    int *new_pixels = (int *) malloc(new_width * new_height * sizeof(int));


        for (int y = 0; y < bmp->height; ++y)
            {
                for (int x = 0; x < bmp->width; ++x)
                {
                    int stored_pixel = bmp->pixels[y * bmp->width + x];
                    new_pixels[x * new_width + y] = stored_pixel;
                }
            }
            
        free(bmp->pixels);
        bmp->pixels = new_pixels;
        bmp->width = new_height;
        bmp->height = new_width;
}

void bitmap_skew(struct bitmap *bmp)
{
    int *new_pixels = (int *) malloc(bmp->width * bmp->height * sizeof(int));

        for (int y = 0; y < bmp->height; ++y)
            {
                for (int x = 0; x < bmp->width; ++x)
                {
                    int i = y * bmp->width + x;
                    int stored_pixel = bmp->pixels[i];

                    new_pixels[i - y] = stored_pixel;
                }
            }
            
        free(bmp->pixels);
        bmp->pixels = new_pixels;
}

void bitmap_shrink(struct bitmap *bmp)
{
    int new_width = bmp->width / 2;
    int new_height = bmp->height / 2;

    int *new_pixels = (int *) malloc(new_width * new_height * sizeof(int));

        for (int y = 0; y < bmp->height;)
            {
                for (int x = 0; x < bmp->width;)
                {
                    int m = (y / 2) * new_width + (x / 2);
                    int n = y * bmp->width + x;
                    int o = y * bmp->width + x + 1;
                    int p = (y + 1) * bmp->width + x;
                    int q = (y + 1) * bmp->width + x + 1;

                    int first_pixel = bmp->pixels[n];
                    int second_pixel = bmp->pixels[o];
                    int third_pixel = bmp->pixels[p];
                    int fourth_pixel = bmp->pixels[q];

                    int r1, g1, b1, r2, g2, b2, r3, g3, b3, r4, g4, b4;
                    
                    pixel_to_rgb(first_pixel, &r1, &g1, &b1);
                    pixel_to_rgb(second_pixel, &r2, &g2, &b2);
                    pixel_to_rgb(third_pixel, &r3, &g3, &b3);
                    pixel_to_rgb(fourth_pixel, &r4, &g4, &b4);


                    int r = (r1 + r2 + r3 + r4) / 4;
                    int g = (g1 + g2 + g3 + g4) / 4;
                    int b = (b1 + b2 + b3 + b4) / 4;
                    int changed_pixel;
                    rgb_to_pixel(&changed_pixel, r, g, b);
                    new_pixels[m] = changed_pixel;
                    //
                    x+= 2;
                }
                y+= 2;
            }
            
            free(bmp->pixels);
            bmp->pixels = new_pixels;
            bmp->width = new_width;
            bmp->height = new_height;
}
