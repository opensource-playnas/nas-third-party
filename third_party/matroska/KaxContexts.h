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
*/
#ifndef LIBMATROSKA_CONTEXTS_H
#define LIBMATROSKA_CONTEXTS_H

#include "KaxTypes.h"
#include "ebml/EbmlElement.h"

using namespace LIBEBML_NAMESPACE;

START_LIBMATROSKA_NAMESPACE

extern const EbmlSemanticContext KaxMatroska_Context;
extern const EbmlSemanticContext KaxSegment_Context;
extern const EbmlSemanticContext KaxAttachments_Context;
extern const EbmlSemanticContext KaxAttached_Context;
extern const EbmlSemanticContext KaxFileDescription_Context;
extern const EbmlSemanticContext KaxFileName_Context;
extern const EbmlSemanticContext KaxMimeType_Context;
extern const EbmlSemanticContext KaxFileData_Context;
extern const EbmlSemanticContext KaxChapters_Context;
extern const EbmlSemanticContext KaxCluster_Context;
extern const EbmlSemanticContext KaxTags_Context;
extern const EbmlSemanticContext KaxTag_Context;
extern const EbmlSemanticContext KaxBlockGroup_Context;
extern const EbmlSemanticContext KaxReferencePriority_Context;
extern const EbmlSemanticContext KaxReferenceBlock_Context;
extern const EbmlSemanticContext KaxReferenceVirtual_Context;
extern const EbmlSemanticContext KaxCues_Context;
extern const EbmlSemanticContext KaxInfo_Context;
extern const EbmlSemanticContext KaxSeekHead_Context;
extern const EbmlSemanticContext KaxTracks_Context;
extern const EbmlSemanticContext KaxTrackEntry_Context;
extern const EbmlSemanticContext KaxTrackNumber_Context;
extern const EbmlSemanticContext KaxTrackType_Context;
extern const EbmlSemanticContext KaxTrackFlagEnabled_Context;
extern const EbmlSemanticContext KaxTrackFlagDefault_Context;
extern const EbmlSemanticContext KaxTrackFlagLacing_Context;
extern const EbmlSemanticContext KaxTrackName_Context;
extern const EbmlSemanticContext KaxTrackLanguage_Context;
extern const EbmlSemanticContext KaxCodecID_Context;
extern const EbmlSemanticContext KaxCodecPrivate_Context;
extern const EbmlSemanticContext KaxCodecName_Context;
extern const EbmlSemanticContext KaxCodecSettings_Context;
extern const EbmlSemanticContext KaxCodecInfoURL_Context;
extern const EbmlSemanticContext KaxCodecDownloadURL_Context;
extern const EbmlSemanticContext KaxCodecDecodeAll_Context;
extern const EbmlSemanticContext KaxTrackOverlay_Context;

#define Context_KaxMatroska              KaxMatroska_Context
#define Context_KaxSegment               KaxSegment_Context
#define Context_KaxTrackEntry            KaxTrackEntry_Context
#define Context_KaxTracks                KaxTracks_Context
#define Context_KaxCluster               KaxCluster_Context
#define Context_KaxBlockGroup            KaxBlockGroup_Context
#define Context_KaxInfo                  KaxInfo_Context
#define Context_KaxAttached              KaxAttached_Context
#define Context_KaxAttachments           KaxAttachments_Context
#define Context_KaxChapters              KaxChapters_Context
#define Context_KaxCues                  KaxCues_Context
#define Context_KaxTags                  KaxTags_Context
#define Context_KaxSeekHead              KaxSeekHead_Context
#define Context_KaxTag                   KaxTag_Context
#define Context_KaxTagCommercial         KaxTagCommercial_Context

extern const EbmlSemanticContext & GetKaxGlobal_Context();
//extern MATROSKA_DLL_API const EbmlSemanticContext & GetKaxTagsGlobal_Context();

END_LIBMATROSKA_NAMESPACE

#endif // LIBMATROSKA_CONTEXTS_H
