/*
 * filter_charcoal.c -- charcoal filter
 * Copyright (C) 2003-2004 Ushodaya Enterprises Limited
 * Author: Charles Yates <charles.yates@pandora.be>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "filter_charcoal.h"

#include <framework/mlt_frame.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static inline int get_Y( uint8_t *pixels, int width, int height, int x, int y )
{
	if ( x < 0 || x >= width || y < 0 || y >= height )
	{
		return 235;
	}
	else
	{
		uint8_t *pixel = pixels + y * ( width << 1 ) + ( x << 1 );
		return *pixel;
	}
}

static inline int sqrti( int n )
{
	int p = 0;
	int q = 1;
	int r = n;
	int h = 0;

	while( q <= n )
		q = q << 2;

	while( q != 1 )
	{
		q = q >> 2;
		h = p + q;
		p = p >> 1;
		if ( r >= h )
		{
			p = p + q;
			r = r - h;
		}
	}

	return p;
}

/** Do it :-).
*/

static int filter_get_image( mlt_frame this, uint8_t **image, mlt_image_format *format, int *width, int *height, int writable )
{
	// Get the filter
	mlt_filter filter = mlt_frame_pop_service( this );

	// Get the image
	int error = mlt_frame_get_image( this, image, format, width, height, 1 );

	// Only process if we have no error and a valid colour space
	if ( error == 0 && *format == mlt_image_yuv422 )
	{
		// Get the charcoal scatter value
		int x_scatter = mlt_properties_get_double( MLT_FILTER_PROPERTIES( filter ), "x_scatter" );
		int y_scatter = mlt_properties_get_double( MLT_FILTER_PROPERTIES( filter ), "y_scatter" );
		float scale = mlt_properties_get_double( MLT_FILTER_PROPERTIES( filter ), "scale" );
		float mix = mlt_properties_get_double( MLT_FILTER_PROPERTIES( filter ), "mix" );
		int invert = mlt_properties_get_int( MLT_FILTER_PROPERTIES( filter ), "invert" );

		// We'll process pixel by pixel
		int x = 0;
		int y = 0;

		// We need to create a new frame as this effect modifies the input
		uint8_t *temp = mlt_pool_alloc( *width * *height * 2 );
		uint8_t *p = temp;
		uint8_t *q = *image;

		// Calculations are carried out on a 3x3 matrix
		int matrix[ 3 ][ 3 ];

		// Used to carry out the matrix calculations
		int sum1;
		int sum2;
		float sum;

		// Loop for each row
		for ( y = 0; y < *height; y ++ )
		{
			// Loop for each pixel
			for ( x = 0; x < *width; x ++ )
			{
				// Populate the matrix
				matrix[ 0 ][ 0 ] = get_Y( *image, *width, *height, x - x_scatter, y - y_scatter );
				matrix[ 0 ][ 1 ] = get_Y( *image, *width, *height, x            , y - y_scatter );
				matrix[ 0 ][ 2 ] = get_Y( *image, *width, *height, x + x_scatter, y - y_scatter );
				matrix[ 1 ][ 0 ] = get_Y( *image, *width, *height, x - x_scatter, y             );
				matrix[ 1 ][ 2 ] = get_Y( *image, *width, *height, x + x_scatter, y             );
				matrix[ 2 ][ 0 ] = get_Y( *image, *width, *height, x - x_scatter, y + y_scatter );
				matrix[ 2 ][ 1 ] = get_Y( *image, *width, *height, x            , y + y_scatter );
				matrix[ 2 ][ 2 ] = get_Y( *image, *width, *height, x + x_scatter, y + y_scatter );

				// Do calculations
				sum1 = (matrix[2][0] - matrix[0][0]) + ( (matrix[2][1] - matrix[0][1]) << 1 ) + (matrix[2][2] - matrix[2][0]);
				sum2 = (matrix[0][2] - matrix[0][0]) + ( (matrix[1][2] - matrix[1][0]) << 1 ) + (matrix[2][2] - matrix[2][0]);
				sum = scale * sqrti( sum1 * sum1 + sum2 * sum2 );

				// Assign value
				*p ++ = !invert ? ( sum >= 16 && sum <= 235 ? 251 - sum : sum < 16 ? 235 : 16 ) :
								  ( sum >= 16 && sum <= 235 ? sum : sum < 16 ? 16 : 235 );
				q ++;
				*p ++ = 128 + mix * ( *q ++ - 128 );
			}
		}

		// Return the created image
		*image = temp;

		// Store new and destroy old
		mlt_properties_set_data( MLT_FRAME_PROPERTIES( this ), "image", *image, *width * *height * 2, mlt_pool_release, NULL );
	}

	return error;
}

/** Filter processing.
*/

static mlt_frame filter_process( mlt_filter this, mlt_frame frame )
{
	// Push the frame filter
	mlt_frame_push_service( frame, this );
	mlt_frame_push_get_image( frame, filter_get_image );

	return frame;
}

/** Constructor for the filter.
*/

mlt_filter filter_charcoal_init( char *arg )
{
	mlt_filter this = mlt_filter_new( );
	if ( this != NULL )
	{
		this->process = filter_process;
		mlt_properties_set( MLT_FILTER_PROPERTIES( this ), "x_scatter", "1" );
		mlt_properties_set( MLT_FILTER_PROPERTIES( this ), "y_scatter", "1" );
		mlt_properties_set( MLT_FILTER_PROPERTIES( this ), "scale", "1.5" );
		mlt_properties_set( MLT_FILTER_PROPERTIES( this ), "mix", "0" );
	}
	return this;
}

