//------------------------------------------------------------------------------
// Copyright (c) 2011-2012 by European Organization for Nuclear Research (CERN)
// Author: Lukasz Janyst <ljanyst@cern.ch>
//-----------------------------------------------------------------------------
// This file is part of the XRootD software suite.
//
// XRootD is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// XRootD is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with XRootD.  If not, see <http://www.gnu.org/licenses/>.
//
// In applying this licence, CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.
//------------------------------------------------------------------------------

#ifndef __XRD_CL_UGLY_HACKS_HH__
#define __XRD_CL_UGLY_HACKS_HH__

#include "XrdSys/XrdSysLinuxSemaphore.hh"
#include "XrdSys/XrdSysPthread.hh"

#if defined(__GLIBC__) && defined(__GLIBC_MINOR__)
#define USE_LIBC_SEMAPHORE (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 21))  // in libc >= 2.21 the semaphore problem has been fixed
#else
#define USE_LIBC_SEMAPHORE 0
#endif

namespace XrdCl
{
#if defined(__linux__) && defined(HAVE_ATOMICS) && !USE_LIBC_SEMAPHORE
  typedef XrdSys::LinuxSemaphore Semaphore;
#else
  typedef XrdSysSemaphore Semaphore;
#endif

#if __cplusplus >= 201103L
#define XRDCL_SMART_PTR_T std::unique_ptr
#else
#define XRDCL_SMART_PTR_T std::auto_ptr
#endif

}

#endif // __XRD_CL_UGLY_HACKS_HH__
