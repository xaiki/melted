/*
 * valerie_notifier.c -- Unit Status Notifier Handling
 * Copyright (C) 2002-2003 Ushodaya Enterprises Limited
 * Author: Charles Yates <charles.yates@pandora.be>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* System header files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

/* Application header files */
#include "valerie_notifier.h"

/** Notifier initialisation.
*/

valerie_notifier valerie_notifier_init( )
{
	valerie_notifier this = malloc( sizeof( valerie_notifier_t ) );
	if ( this != NULL )
	{
		int index = 0;
		memset( this, 0, sizeof( valerie_notifier_t ) );
		pthread_mutex_init( &this->mutex, NULL );
		pthread_cond_init( &this->cond, NULL );
		pthread_mutex_init( &this->cond_mutex, NULL );
		for ( index = 0; index < MAX_UNITS; index ++ )
			this->store[ index ].unit = index;
	}
	return this;
}

/** Get a stored status for the specified unit.
*/

void valerie_notifier_get( valerie_notifier this, valerie_status status, int unit )
{
	pthread_mutex_lock( &this->mutex );
	if ( unit >= 0 && unit < MAX_UNITS )
	{
		valerie_status_copy( status, &this->store[ unit ] );
	}
	else
	{
		memset( status, 0, sizeof( valerie_status_t ) );
		status->unit = unit;
	}
	status->dummy = time( NULL );
	pthread_mutex_unlock( &this->mutex );
}

/** Wait on a new status.
*/

int valerie_notifier_wait( valerie_notifier this, valerie_status status )
{
	struct timeval now;
	struct timespec timeout;
	int error = 0;

	memset( status, 0, sizeof( valerie_status_t ) );

	pthread_mutex_lock( &this->cond_mutex );
	gettimeofday( &now, NULL );
	timeout.tv_sec = now.tv_sec + 1;
	timeout.tv_nsec = now.tv_usec * 1000;
	if ( pthread_cond_timedwait( &this->cond, &this->cond_mutex, &timeout ) != ETIMEDOUT )
	{
		pthread_mutex_lock( &this->mutex );
		valerie_status_copy( status, &this->last );
		pthread_mutex_unlock( &this->mutex );
	}
	else
	{
		error = -1;
	}
	pthread_mutex_unlock( &this->cond_mutex );

	return error;
}

/** Put a new status.
*/

void valerie_notifier_put( valerie_notifier this, valerie_status status )
{
	pthread_mutex_lock( &this->mutex );
	valerie_status_copy( &this->store[ status->unit ], status );
	valerie_status_copy( &this->last, status );
	pthread_mutex_unlock( &this->mutex );
	pthread_cond_broadcast( &this->cond );
}

/** Communicate a disconnected status for all units to all waiting.
*/

void valerie_notifier_disconnected( valerie_notifier notifier )
{
	int unit = 0;
	valerie_status_t status;
	for ( unit = 0; unit < MAX_UNITS; unit ++ )
	{
		valerie_notifier_get( notifier, &status, unit );
		status.status = unit_disconnected;
		valerie_notifier_put( notifier, &status );
	}
}

/** Close the notifier - note that all access must be stopped before we call this.
*/

void valerie_notifier_close( valerie_notifier this )
{
	if ( this != NULL )
	{
		pthread_mutex_destroy( &this->mutex );
		pthread_mutex_destroy( &this->cond_mutex );
		pthread_cond_destroy( &this->cond );
		free( this );
	}
}
