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

#ifndef CMDDUP_H_INCLUDED
#define CMDDUP_H_INCLUDED

#include <errno.h>
#include <limits.h>

static bool resdup_newACR( FILE * stream );

static bool resdup_listAcro( HWND hwnd, FILE * stream );

static bool compDesc   ( wchar_t const * const desc_acro, wchar_t const * const desc_dup );
static int8_t lookAtDef( wchar_t const * const desc_dup,  wchar_t const * const desc_acro );

static bool resdup_findAcroOcc( FILE * stream,
                                wchar_t const * const acro,
                                wchar_t const * const desc_acro,
                                uint32_t const desc_stream_pos );

static int32_t cmpr( const void * a, const void * b );

static void renameFile( HWND hwnd, wchar_t * filename );

static void fullDelDuplAcr( FILE * stream, wchar_t const * const dup,
                                           wchar_t const * const desc_dup,
                                           uint32_t const desc_stream_pos );

#endif // CMDDUP_H_INCLUDED
