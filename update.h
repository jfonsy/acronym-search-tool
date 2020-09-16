/*
 * Copyright (©) 2019 J. Fonseka
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UPDATE_H_INCLUDED
#define UPDATE_H_INCLUDED

bool add_occurred = false;

static bool startCompare( HWND hwnd, FILE * stream_loc, FILE * stream_net,
                         uint32_t const stream_loc_entries,
                         uint32_t const stream_net_entries );

static bool beginTFOSC  ( HWND hwnd, FILE * fptr_loc, FILE * fptr_net,
                         wchar_t * filecont_loc,
                         wchar_t * filecont_net );

static wchar_t * getLastEntry  ( FILE * stream, uint32_t file_size );

static inline int32_t chkSum( uint32_t file_size, wchar_t * filecont );

static void upd_addToFile( HWND hwnd, FILE * fptr_loc, wchar_t * to_add );


#endif // UPDATE_H_INCLUDED
