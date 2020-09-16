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

#ifndef LOAD_H_INCLUDED
#define LOAD_H_INCLUDED

static AcronymDB * mapAcroDefs( wchar_t * filecont );
static AcronymDB * initAcroDB( uint32_t rows );

static void addDefn( AcronymDB * entry_list,
                       wchar_t * delimbuff,
                       uint32_t r );

static void addAcronym( AcronymDB * entry_list,
                          wchar_t * delimbuff,
                          uint32_t r );

#endif // LOAD_H_INCLUDED
