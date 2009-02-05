/*
 *  SQLInstallDriverEx.c
 *
 *  $Id: SQLInstallDriverEx.c,v 1.3 2004/11/11 01:52:40 luesang Exp $
 *
 *  These functions intentionally left blank
 *
 *  The iODBC driver manager.
 *  
 *  Copyright (C) 1999-2002 by OpenLink Software <iodbc@openlinksw.com>
 *  All Rights Reserved.
 *
 *  This software is released under the terms of either of the following
 *  licenses:
 *
 *      - GNU Library General Public License (see LICENSE.LGPL) 
 *      - The BSD License (see LICENSE.BSD).
 *
 *  While not mandated by the BSD license, any patches you make to the
 *  iODBC source code may be contributed back into the iODBC project
 *  at your discretion. Contributions will benefit the Open Source and
 *  Data Access community as a whole. Submissions may be made at:
 *
 *      http://www.iodbc.org
 *
 *
 *  GNU Library Generic Public License Version 2
 *  ============================================
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 *  The BSD License
 *  ===============
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *  3. Neither the name of OpenLink Software Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL OPENLINK OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iodbc.h>
#include <iodbcinst.h>

#include "misc.h"
#include "inifile.h"
#include "iodbc_error.h"

#ifdef _MAC
# include <getfpn.h>
#endif

#if !defined(WINDOWS) && !defined(WIN32) && !defined(OS2) && !defined(macintosh)
# include <pwd.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>
# define UNIX_PWD
#endif

extern BOOL InstallDriverPath ( LPSTR lpszPath,WORD cbPathMax,
    WORD * pcbPathOut,LPSTR envname);


BOOL
InstallDriverPathLength (WORD *pcbPathOut, LPSTR envname)
{
#ifdef _MAC
  OSErr result;
  long fldrDid;
  short fldrRef;
#endif
  BOOL retcode = FALSE;
  WORD len = 0;
  char path[1024];
  char *ptr;

#if	!defined(UNIX_PWD)
#ifdef _MAC
  result = FindFolder (kOnSystemDisk, kExtensionFolderType, kDontCreateFolder,
      &fldrRef, &fldrDid);
  if (result != noErr)
    {
      PUSH_ERROR (ODBC_ERROR_GENERAL_ERR);
      goto quit;
    }

  ptr = get_full_pathname (fldrDid, fldrRef);
  len = STRLEN (ptr);
  free (ptr);

  goto done;
#else
  /*
     *  On Windows, there is only one place to look
   */
  len = GetWindowsDirectory (path, sizeof (path));
  goto done;
#endif
#else

  /*
   *  1. Check $ODBCDRIVERS environment variable
   */
  if ((ptr = getenv (envname)) && access (ptr, R_OK | W_OK | X_OK) == 0)
    {
      len = STRLEN (ptr);
      goto done;
    }

  /*
   * 2. Check /usr/local/lib and /usr/lib
   */
#ifdef _BE
  if (access ("/boot/beos/system/lib", R_OK | W_OK | X_OK) == 0)
    {
      len = STRLEN ("/boot/beos/system/lib");
      goto done;
    }
#else
  if (access ("/usr/local/lib", R_OK | W_OK | X_OK) == 0)
    {
      len = STRLEN ("/usr/local/lib");
      goto done;
    }
#endif

#ifdef _BE
  if (access ("/boot/home/config/lib", R_OK | W_OK | X_OK) == 0)
    {
      len = STRLEN ("/boot/home/config/lib");
      goto done;
    }
#else
  if (access ("/usr/lib", R_OK | W_OK | X_OK) == 0)
    {
      len = STRLEN ("/usr/lib");
      goto done;
    }
#endif

  /*
   *  3. Check either $HOME
   */
  if (!(ptr = getenv ("HOME")))
    {
      ptr = (char *) getpwuid (getuid ());
      if (ptr)
	ptr = ((struct passwd *) ptr)->pw_dir;
    }

  if (ptr)
    {
#ifdef _BE
      sprintf (path, "%s/config/lib", ptr);
#else
      sprintf (path, "%s/lib", ptr);
#endif
      if (access (path, R_OK | W_OK | X_OK) == 0)
	{
	  len = STRLEN (path);
	  goto done;
	}
    }

  if (!mkdir (path, 0755))
    goto done;

#endif

  SQLPostInstallerError (ODBC_ERROR_GENERAL_ERR,
      "Cannot retrieve a directory where to install the driver or translator.");
  goto quit;

done:
  retcode = TRUE;

quit:
  if (pcbPathOut)
    *pcbPathOut = len;
  return retcode;
}


BOOL INSTAPI
SQLInstallDriverEx (LPCSTR lpszDriver, LPCSTR lpszPathIn, LPSTR lpszPathOut,
    WORD cbPathOutMax, WORD *pcbPathOut, WORD fRequest, LPDWORD lpdwUsageCount)
{
  PCONFIG pCfg = NULL, pOdbcCfg = NULL;
  BOOL retcode = FALSE;

  CLEAR_ERROR ();

  if (lpszPathIn && access (lpszPathIn, R_OK | W_OK | X_OK))
    {
      PUSH_ERROR (ODBC_ERROR_INVALID_PATH);
      goto quit;
    }

  switch (fRequest)
    {
    case ODBC_INSTALL_INQUIRY:
      if (lpszPathIn)
	{
	  if (pcbPathOut)
	    *pcbPathOut = STRLEN (lpszPathIn);
	  retcode = TRUE;
	}
      else
	retcode = InstallDriverPathLength (pcbPathOut, "ODBCDRIVERS");
      goto quit;

    case ODBC_INSTALL_COMPLETE:
      break;

    default:
      PUSH_ERROR (ODBC_ERROR_INVALID_REQUEST_TYPE);
      goto quit;
    };

  /* Check input parameters */
  if (!lpszDriver || !STRLEN (lpszDriver))
    {
      PUSH_ERROR (ODBC_ERROR_INVALID_PARAM_SEQUENCE);
      goto quit;
    }

  if (!lpszPathOut || !cbPathOutMax)
    {
      PUSH_ERROR (ODBC_ERROR_INVALID_BUFF_LEN);
      goto quit;
    }

  /* Write the out path */
  if (!InstallDriverPath (lpszPathOut, cbPathOutMax, pcbPathOut,
	  "ODBCDRIVERS"))
    goto quit;

  /* Else go through user/system odbcinst.ini */
  switch (configMode)
    {
    case ODBC_BOTH_DSN:
    case ODBC_USER_DSN:
      wSystemDSN = USERDSN_ONLY;
      break;

    case ODBC_SYSTEM_DSN:
      wSystemDSN = SYSTEMDSN_ONLY;
      break;
    };

  if (_iodbcdm_cfg_search_init (&pCfg, "odbcinst.ini", TRUE))
    {
      PUSH_ERROR (ODBC_ERROR_GENERAL_ERR);
      goto quit;
    }

  if (_iodbcdm_cfg_search_init (&pOdbcCfg, "odbc.ini", TRUE))
    {
      PUSH_ERROR (ODBC_ERROR_GENERAL_ERR);
      pOdbcCfg = NULL;
      goto done;
    }

  if (!install_from_string (pCfg, pOdbcCfg, (char *) lpszDriver, TRUE))
    {
      PUSH_ERROR (ODBC_ERROR_INVALID_KEYWORD_VALUE);
      goto done;
    }

  if (_iodbcdm_cfg_commit (pCfg) || _iodbcdm_cfg_commit (pOdbcCfg))
    {
      PUSH_ERROR (ODBC_ERROR_GENERAL_ERR);
      goto done;
    }

  retcode = TRUE;

done:
  _iodbcdm_cfg_done (pCfg);
  if (pOdbcCfg)
    _iodbcdm_cfg_done (pOdbcCfg);

quit:
  wSystemDSN = USERDSN_ONLY;
  configMode = ODBC_BOTH_DSN;

  return retcode;
}
