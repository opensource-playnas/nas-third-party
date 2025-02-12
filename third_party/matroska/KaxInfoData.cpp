/****************************************************************************
** libmatroska : parse Matroska files, see http://www.matroska.org/
**
** <file/class description>
**
** Copyright (C) 2002-2010 Steve Lhomme.  All rights reserved.
**
** This file is part of libmatroska.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
** See http://www.gnu.org/licenses/lgpl-2.1.html for LGPL licensing information.**
** Contact license@matroska.org if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

/*!
  \file
  \version \$Id$
  \author Steve Lhomme     <robux4 @ users.sf.net>
  \author John Cannon      <spyder2555 @ users.sf.net>
*/
#include "KaxInfoData.h"
#include "KaxContexts.h"
#include "KaxDefines.h"
#include "KaxSemantic.h"

START_LIBMATROSKA_NAMESPACE

#if defined(HAVE_EBML2) || defined(HAS_EBML2)
KaxSegmentUID::KaxSegmentUID(EBML_DEF_CONS EBML_DEF_SEP EBML_EXTRA_DEF)
  :EbmlBinary(EBML_DEF_PARAM EBML_DEF_SEP EBML_EXTRA_CALL)
{
}
#endif

END_LIBMATROSKA_NAMESPACE
