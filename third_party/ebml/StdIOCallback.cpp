/****************************************************************************
** libebml : parse EBML files, see http://embl.sourceforge.net/
**
** <file/class description>
**
** Copyright (C) 2002-2004 Ingo Ralf Blum.  All rights reserved.
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
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
**
** See http://www.gnu.org/licenses/lgpl-2.1.html for LGPL licensing information.
**
** Contact license@matroska.org if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

/*!
  \file
  \version \$Id: StdIOCallback.cpp 1298 2008-02-21 22:14:18Z mosu $
  \author Steve Lhomme     <robux4 @ users.sf.net>
  \author Moritz Bunkus <moritz @ bunkus.org>
*/

#include <cassert>
#include <climits>
#include <sstream>

#include "build/build_config.h"

#include "Debug.h"
#include "EbmlConfig.h"
#include "StdIOCallback.h"

#include "base/strings/sys_string_conversions.h"
#include "base/strings/utf_string_conversions.h"

using namespace std;

START_LIBEBML_NAMESPACE

CRTError::CRTError(int nError, const std::string& Description)
    : std::runtime_error(Description + ": " + strerror(nError)),
      Error(nError) {}

CRTError::CRTError(const std::string& Description, int nError)
    : std::runtime_error(Description + ": " + strerror(nError)),
      Error(nError) {}

StdIOCallback::StdIOCallback(const base::FilePath& Path,
                             const open_mode aMode) {
  base::FilePath::StringType Mode;
  switch (aMode) {
    case MODE_READ:
      Mode = FILE_PATH_LITERAL("rb");
      break;
    case MODE_SAFE:
      Mode = FILE_PATH_LITERAL("rb+");
      break;
    case MODE_WRITE:
      Mode = FILE_PATH_LITERAL("wb");
      break;
    case MODE_CREATE:
      Mode = FILE_PATH_LITERAL("wb+");
      break;
    default:
      throw 0;
  }
#if BUILDFLAG(IS_WIN)
  std::wstring temp_str = base::UTF8ToWide(Path.AsUTF8Unsafe());
  File = _wfopen(temp_str.c_str(), Mode.c_str());
#else
  File = fopen(Path.AsUTF8Unsafe().c_str(), Mode.c_str());
#endif
  if (File == nullptr) {
    stringstream Msg;
    Msg << "Can't open stdio file \"" << Path << "\" in mode \"" << Mode.c_str()
        << "\"";
    throw CRTError(Msg.str());
  }
  mCurrentPosition = 0;
}

StdIOCallback::~StdIOCallback() noexcept {
  close();
}

uint32 StdIOCallback::read(void* Buffer, size_t Size) {
  assert(File != nullptr);

  size_t result = fread(Buffer, 1, Size, File);
  mCurrentPosition += result;
  return result;
}

void StdIOCallback::setFilePointer(int64 Offset, seek_mode Mode) {
  assert(File != nullptr);

  // There is a numeric cast in the boost library, which would be quite nice for
  // this checking
  /*
    SL : replaced because unknown class in cygwin
    assert(Offset <= numeric_limits<long>::max());
    assert(Offset >= numeric_limits<long>::min());
  */

  /*assert(Offset <= LONG_MAX);
  assert(Offset >= LONG_MIN);*/
  assert(Offset <= (numeric_limits<int64>::max)());
  assert(Offset >= (numeric_limits<int64>::min)());

  assert(Mode == SEEK_CUR || Mode == SEEK_END || Mode == SEEK_SET);

#if BUILDFLAG(IS_WIN)
  if (_fseeki64(File, Offset, Mode) != 0) {
#else
  if (fseeko64(File, Offset, Mode) != 0) {
#endif
    ostringstream Msg;
    Msg << "Failed to seek file " << File << " to offset "
        << static_cast<unsigned long long>(Offset) << " in mode " << Mode;
    throw CRTError(Msg.str());
  } else {
    switch (Mode) {
      case SEEK_CUR:
        mCurrentPosition += Offset;
        break;
      case SEEK_END:
#if BUILDFLAG(IS_WIN)
        mCurrentPosition = _ftelli64(File);
#else
        mCurrentPosition = ftello(File);
#endif
        break;
      case SEEK_SET:
        mCurrentPosition = Offset;
        break;
    }
  }
}

size_t StdIOCallback::write(const void* Buffer, size_t Size) {
  assert(File != nullptr);
  uint32 Result = fwrite(Buffer, 1, Size, File);
  mCurrentPosition += Result;
  return Result;
}

uint64 StdIOCallback::getFilePointer() {
  assert(File != nullptr);

#if 0
  long Result=ftell(File);
  if(Result<0) {
    stringstream Msg;
    Msg<<"Can't tell the current file pointer position for "<<File;
    throw CRTError(Msg.str());
  }
#endif

  return mCurrentPosition;
}

void StdIOCallback::close() {
  if (File == nullptr)
    return;

  if (fclose(File) != 0) {
    stringstream Msg;
    Msg << "Can't close file " << File;
    throw CRTError(Msg.str());
  }

  File = nullptr;
}

END_LIBEBML_NAMESPACE
