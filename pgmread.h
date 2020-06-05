#ifndef PGM_READ_H
#define PGM_READ_H

struct Image
{
	int width;
	int height;
	unsigned char* data;
};

/* Image_create takes a buffer that contains all the bytes from a PGM
 * image of type P2. It creates a struct Image and fills in the width
 * and height information as well as the data. The buffer remains
 ' unchanged.
 */
struct Image* Image_create( char* buffer );

/* Image_alloc is used by Image_create to allocate the memory for
 * a struct Image and the image data contains in it.
 */
struct Image* Image_alloc( int w, int h );

/* Image_free releases the memory of a struct Image and the data
 * that contains the image pixels.
 */
void Image_free( struct Image* img );

/* Image_compare takes to struct Image pointers and compares them.
 * It returns 0 if the images are different, and 1 if they are identical.
 * It prints a warning before returning 0 if they have different width
 * or height, or if of the pointers is NULL.
 */
int Image_compare( struct Image* img1, struct Image* img2 );

#endif /* PGM_READ_H */
