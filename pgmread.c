#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "pgmread.h"


struct Image* Image_alloc( int w, int h )
{
	struct Image* img = malloc( sizeof(struct Image) );
	img->width = w;
	img->height = h;
	img->data = malloc( sizeof(unsigned char) * w * h );
	return img;
}

void Image_free( struct Image* img )
{
	if(img==NULL) return;
	if(img->data) free(img->data);
	free(img);
}

int Image_compare( struct Image* img1, struct Image* img2 )
{
	int w, h;
	if( img1 == NULL || img2 == NULL )
	{
		//fprintf(stderr, "WARNING: one or both of images are NULL\n");
		return 0;
	}
	if( img1->width != img2->width )
	{
		//fprintf(stderr, "WARNING: image 1 has width %d, image 2 has width %d\n", img1->width, img2->width);
		return 0;
	}
	if( img1->height != img2->height )
	{
		//fprintf(stderr, "WARNING: image 1 has height %d, image 2 has height %d\n", img1->height, img2->height);
		return 0;
	}
	for( h=0; h<img1->height; h++ )
	{
		for( w=0; w<img1->width; w++ )
		{
			if( img1->data[h*img1->width+w] != img2->data[h*img1->width+w] )
				return 0;
		}
	}
	return 1;
}

struct Image* readPGMfile( char* buffer )
{
	char* ptr;
	char* line;
	char* ctx = NULL;
	struct Image* image;
	int width;
	int height;
	int ret;
	int w,h;

	ptr = buffer;
	while( ptr != NULL && ( isspace(*ptr) ) ) ptr++;

	if( ptr == NULL )
	{
		//fprintf(stderr, "WARNING: No content in image buffer\n" );
		return NULL;
	}

	line = strtok_r( ptr, "\n", &ctx );
	////fprintf( stderr, "%s\n", line );

	if( line == NULL )
	{
		//fprintf(stderr, "WARNING: No header in image buffer\n" );
		return NULL;
	}

	if( strstr( line, "P2" ) == NULL )
	{
		//fprintf(stderr, "WARNING: Image is not a P2-style PGM image\n" );
		return NULL;
	}

	line = strtok_r( NULL, "\n", &ctx );
	// ////fprintf( stderr, "%s\n", line );

	ret = sscanf( line, "%d %d", &width, &height );
	if( ret != 2 )
	{
		//fprintf(stderr, "WARNING: P2 image does not contain width and height on line 2\n" );
		return NULL;
	}

	line = strtok_r( NULL, "\n", &ctx );
	////fprintf( stderr, "%s\n", line );
	if( strcmp( "255", line ) )
	{
		//fprintf(stderr, "WARNING: P2 image is not 1-byte-encoded\n");
		return NULL;
	}

	image = Image_alloc( width, height );

	for( h=0; h<height; h++ )
	{
		for( w=0; w<width; w++ )
		{
			line = strtok_r( NULL, " \n", &ctx );
			int input = atoi( line );
			// ////fprintf(stderr, "%d ", input);
			image->data[h*image->width+w] = input;
		}
		// ////fprintf(stderr, "\n");
	}

	return image;
}

