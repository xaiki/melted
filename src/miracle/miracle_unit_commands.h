/*
 * unit_commands.h
 * Copyright (C) 2002-2003 Ushodaya Enterprises Limited
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


#ifndef _UNIT_COMMANDS_H_
#define _UNIT_COMMANDS_H_

#include "dvconnection.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern response_codes dv1394d_list( command_argument );
extern response_codes dv1394d_load( command_argument );
extern response_codes dv1394d_insert( command_argument );
extern response_codes dv1394d_remove( command_argument );
extern response_codes dv1394d_clean( command_argument );
extern response_codes dv1394d_move( command_argument );
extern response_codes dv1394d_append( command_argument );
extern response_codes dv1394d_play( command_argument );
extern response_codes dv1394d_stop( command_argument );
extern response_codes dv1394d_pause( command_argument );
extern response_codes dv1394d_rewind( command_argument );
extern response_codes dv1394d_step( command_argument );
extern response_codes dv1394d_goto( command_argument );
extern response_codes dv1394d_ff( command_argument );
extern response_codes dv1394d_set_in_point( command_argument );
extern response_codes dv1394d_set_out_point( command_argument );
extern response_codes dv1394d_get_unit_status( command_argument );
extern response_codes dv1394d_set_unit_property( command_argument );
extern response_codes dv1394d_get_unit_property( command_argument );
extern response_codes dv1394d_transfer( command_argument );

#ifdef __cplusplus
}
#endif

#endif
