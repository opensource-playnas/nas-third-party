/****************************************************************************
** libebml : parse EBML files, see http://embl.sourceforge.net/
**
** <file/class description>
**
** Copyright (C) 2002-2010 Steve Lhomme.  All rights reserved.
**
** This file is part of libebml.
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
** See http://www.gnu.org/licenses/lgpl-2.1.html for LGPL licensing information.
**
** Contact license@matroska.org if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

/*!
  \file
  \version \$Id$
  \author Steve Lhomme     <robux4 @ users.sf.net>
  \author Julien Coloos    <suiryc @ users.sf.net>
  \author Moritz Bunkus    <moritz @ bunkus.org>
*/
#ifndef LIBEBML_UINTEGER_H
#define LIBEBML_UINTEGER_H

#include "EbmlTypes.h"
#include "EbmlElement.h"

START_LIBEBML_NAMESPACE

const int DEFAULT_UINT_SIZE = 0; ///< optimal size stored

/*!
    \class EbmlUInteger
    \brief Handle all operations on an unsigned integer EBML element
*/
class EbmlUInteger : public EbmlElement {
  public:
    EbmlUInteger();
    EbmlUInteger(uint64 DefaultValue);
    EbmlUInteger(const EbmlUInteger & ElementToClone) = default;

    EbmlUInteger & operator=(uint64 NewValue) {Value = NewValue; SetValueIsSet(); return *this;}

    /*!
      Set the default size of the integer (usually 1,2,4 or 8)
    */
    virtual void SetDefaultSize(uint64 nDefaultSize = DEFAULT_UINT_SIZE) {EbmlElement::SetDefaultSize(nDefaultSize); SetSize_(nDefaultSize);}

    virtual bool ValidateSize() const {return IsFiniteSize() && (GetSize() <= 8);}
    filepos_t RenderData(IOCallback & output, bool bForceRender, bool bWithDefault = false);
    filepos_t ReadData(IOCallback & input, ScopeMode ReadFully = SCOPE_ALL_DATA);
    filepos_t UpdateSize(bool bWithDefault = false, bool bForceRender = false);

    virtual bool IsSmallerThan(const EbmlElement *Cmp) const;

    operator uint8()  const;
    operator uint16() const;
    operator uint32() const;
    operator uint64() const;

    EbmlUInteger &SetValue(uint64 NewValue);
    uint64 GetValue() const;

    void SetDefaultValue(uint64);

    uint64 DefaultVal() const;

    bool IsDefaultValue() const {
      return (DefaultISset() && Value == DefaultValue);
    }

#if defined(EBML_STRICT_API)
    private:
#else
    protected:
#endif
    uint64 Value; /// The actual value of the element
    uint64 DefaultValue;
};

END_LIBEBML_NAMESPACE

#endif // LIBEBML_UINTEGER_H
