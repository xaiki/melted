/*
 * transition_mix.c -- mix two audio streams
 * Copyright (C) 2003-2004 Ushodaya Enterprises Limited
 * Author: Dan Dennedy <dan@dennedy.org>
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

#include "transition_mix.h"
#include <framework/mlt_frame.h>

#include <stdio.h>
#include <stdlib.h>


/** Get the audio.
*/

static int transition_get_audio( mlt_frame frame, int16_t **buffer, mlt_audio_format *format, int *frequency, int *channels, int *samples )
{
	// Get the properties of the a frame
	mlt_properties a_props = MLT_FRAME_PROPERTIES( frame );

	// Get the b frame from the stack
	mlt_frame b_frame = mlt_frame_pop_audio( frame );

	// Get the properties of the b frame
	mlt_properties b_props = MLT_FRAME_PROPERTIES( b_frame );

	// Restore the original get_audio
	frame->get_audio = mlt_properties_get_data( a_props, "mix.get_audio", NULL );

	double mix_start = 0.5, mix_end = 0.5;
	if ( mlt_properties_get( b_props, "audio.previous_mix" ) != NULL )
		mix_start = mlt_properties_get_double( b_props, "audio.previous_mix" );
	if ( mlt_properties_get( b_props, "audio.mix" ) != NULL )
		mix_end = mlt_properties_get_double( b_props, "audio.mix" );
	if ( mlt_properties_get_int( b_props, "audio.reverse" ) )
	{
		mix_start = 1 - mix_start;
		mix_end = 1 - mix_end;
	}

	mlt_frame_mix_audio( frame, b_frame, mix_start, mix_end, buffer, format, frequency, channels, samples );

	return 0;
}


/** Mix transition processing.
*/

static mlt_frame transition_process( mlt_transition this, mlt_frame a_frame, mlt_frame b_frame )
{
	mlt_properties properties = MLT_TRANSITION_PROPERTIES( this );
	mlt_properties b_props = MLT_FRAME_PROPERTIES( b_frame );

	// Only if mix is specified, otherwise a producer may set the mix
	if ( mlt_properties_get( properties, "start" ) != NULL )
	{
		// Determine the time position of this frame in the transition duration
		mlt_position in = mlt_transition_get_in( this );
		mlt_position out = mlt_transition_get_out( this );
		mlt_position time = mlt_frame_get_position( b_frame );
		double mix = ( double )( time - in ) / ( double )( out - in + 1 );
		
		// If there is an end mix level adjust mix to the range
		if ( mlt_properties_get( properties, "end" ) != NULL )
		{
			double start = mlt_properties_get_double( properties, "start" );
			double end = mlt_properties_get_double( properties, "end" );
			mix = start + ( end - start ) * mix;
		}
		// A negative means total crossfade (uses position)
		else if ( mlt_properties_get_double( properties, "start" ) >= 0 )
		{
			// Otherwise, start/constructor is a constant mix level
		    mix = mlt_properties_get_double( properties, "start" );
		}
	
		// Finally, set the mix property on the frame
		mlt_properties_set_double( b_props, "audio.mix", mix );

		// Initialise transition previous mix value to prevent an inadvertant jump from 0
		if ( mlt_properties_get( properties, "previous_mix" ) == NULL )
			mlt_properties_set_double( properties, "previous_mix", mlt_properties_get_double( b_props, "audio.mix" ) );
			
		// Tell b frame what the previous mix level was
		mlt_properties_set_double( b_props, "audio.previous_mix", mlt_properties_get_double( properties, "previous_mix" ) );

		// Save the current mix level for the next iteration
		mlt_properties_set_double( properties, "previous_mix", mlt_properties_get_double( b_props, "audio.mix" ) );
		
		mlt_properties_set_double( b_props, "audio.reverse", mlt_properties_get_double( properties, "reverse" ) );
	}

	// Ensure that the tractor knows this isn't test audio...
	if ( mlt_properties_get_int( MLT_FRAME_PROPERTIES( a_frame ), "test_audio" ) )
	{
		mlt_properties_set_int( MLT_FRAME_PROPERTIES( a_frame ), "test_audio", 0 );
		mlt_properties_set_int( MLT_FRAME_PROPERTIES( a_frame ), "silent_audio", 1 );
	}

	// Backup the original get_audio (it's still needed)
	mlt_properties_set_data( MLT_FRAME_PROPERTIES( a_frame ), "mix.get_audio", a_frame->get_audio, 0, NULL, NULL );

	// Override the get_audio method
	a_frame->get_audio = transition_get_audio;
	
	mlt_frame_push_audio( a_frame, b_frame );
	
	return a_frame;
}

/** Constructor for the transition.
*/

mlt_transition transition_mix_init( char *arg )
{
	mlt_transition this = calloc( sizeof( struct mlt_transition_s ), 1 );
	if ( this != NULL && mlt_transition_init( this, NULL ) == 0 )
	{
		this->process = transition_process;
		if ( arg != NULL )
			mlt_properties_set_double( MLT_TRANSITION_PROPERTIES( this ), "start", atof( arg ) );
		mlt_properties_set_int( MLT_TRANSITION_PROPERTIES( this ), "_accepts_blanks", 1 );
	}
	return this;
}

