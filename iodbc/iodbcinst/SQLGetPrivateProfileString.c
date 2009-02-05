/*
 *  SQLGetProfileString.c
 *
 *  $Id: SQLGetPrivateProfileString.c,v 1.3 2004/11/11 01:52:40 luesang Exp $
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

#include "inifile.h"
#include "iodbc_error.h"
#include "misc.h"

#ifndef WIN32
int
GetPrivateProfileString (LPCSTR lpszSection, LPCSTR lpszEntry,
    LPCSTR lpszDefault, LPSTR lpszRetBuffer, int cbRetBuffer,
    LPCSTR lpszFilename)
{
  char *defval = (char *) lpszDefault, *value = NULL;
  int len = 0;
  PCONFIG pCfg;

  lpszRetBuffer[0] = 0;

  /* If error during reading the file */
  if (_iodbcdm_cfg_search_init (&pCfg, lpszFilename, FALSE))
    {
      PUSH_ERROR (ODBC_ERROR_INVALID_PATH);
      goto fail;
    }

  /* List all sections from the ini file */
  if (lpszSection == NULL || *lpszSection == '\0')
    {
      len = _iodbcdm_list_sections (pCfg, lpszRetBuffer, cbRetBuffer);
      goto done;
    }

  /* List all the entries of the specified section */
  if (lpszEntry == NULL || *lpszEntry == '\0')
    {
      len =
	  _iodbcdm_list_entries (pCfg, lpszSection, lpszRetBuffer,
	  cbRetBuffer);
      goto done;
    }

  /*
   *  Sorry for this one -- Windows cannot handle a default value of
   *  "" in GetPrivateProfileString, so it is passed as " " instead.
   */
  if (defval == NULL || *defval == '\0')
    defval = " ";

  /*
   *  Check whether someone else has modified the odbc.ini file
   */
  _iodbcdm_cfg_refresh (pCfg);

  if (!_iodbcdm_cfg_find (pCfg, (LPSTR) lpszSection, (LPSTR) lpszEntry))
    value = pCfg->value;

  if (value == NULL)
    {
      value = defval;

      if (value[0] == ' ' && value[1] == '\0')
	value = "";
    }

  STRNCPY (lpszRetBuffer, value, cbRetBuffer - 1);

done:
  _iodbcdm_cfg_done (pCfg);

fail:
  if (!len)
    len = STRLEN (lpszRetBuffer);

  if (len == cbRetBuffer - 1)
    PUSH_ERROR (ODBC_ERROR_INVALID_BUFF_LEN);

  return len;
}
#endif


int INSTAPI
SQLGetPrivateProfileString (LPCSTR lpszSection, LPCSTR lpszEntry,
    LPCSTR lpszDefault, LPSTR lpszRetBuffer, int cbRetBuffer,
    LPCSTR lpszFilename)
{
  char pathbuf[1024];
  int len = 0;

  /* Check input parameters */
  CLEAR_ERROR ();

  if (!lpszRetBuffer || !cbRetBuffer)
    {
      PUSH_ERROR (ODBC_ERROR_INVALID_BUFF_LEN);
      goto quit;
    }

  if (!lpszDefault)
    {
      PUSH_ERROR (ODBC_ERROR_GENERAL_ERR);
      goto quit;
    }

  /* Else go through user/system odbc.ini */
  switch (configMode)
    {
    case ODBC_USER_DSN:
      wSystemDSN = USERDSN_ONLY;
      if (lpszFilename)
	{
	  len =
	      GetPrivateProfileString (lpszSection, lpszEntry, lpszDefault,
	      lpszRetBuffer, cbRetBuffer, lpszFilename);
	  goto quit;
	}
      if (_iodbcadm_getinifile (pathbuf, sizeof (pathbuf), FALSE, FALSE))
	len =
	    GetPrivateProfileString (lpszSection, lpszEntry, lpszDefault,
	    lpszRetBuffer, cbRetBuffer, pathbuf);
      goto quit;

    case ODBC_SYSTEM_DSN:
      wSystemDSN = SYSTEMDSN_ONLY;
      if (lpszFilename)
	{
	  len =
	      GetPrivateProfileString (lpszSection, lpszEntry, lpszDefault,
	      lpszRetBuffer, cbRetBuffer, lpszFilename);
	  goto quit;
	}
      if (_iodbcadm_getinifile (pathbuf, sizeof (pathbuf), FALSE, FALSE))
	len =
	    GetPrivateProfileString (lpszSection, lpszEntry, lpszDefault,
	    lpszRetBuffer, cbRetBuffer, pathbuf);
      goto quit;

    case ODBC_BOTH_DSN:
      wSystemDSN = USERDSN_ONLY;
      if (lpszFilename)
	{
	  len =
	      GetPrivateProfileString (lpszSection, lpszEntry, lpszDefault,
	      lpszRetBuffer, cbRetBuffer, lpszFilename);
	  if (!len)
	    {
	      CLEAR_ERROR ();
	      wSystemDSN = SYSTEMDSN_ONLY;
	      len =
		  GetPrivateProfileString (lpszSection, lpszEntry,
		  lpszDefault, lpszRetBuffer, cbRetBuffer, lpszFilename);
	    }
	  goto quit;
	}
      if (_iodbcadm_getinifile (pathbuf, sizeof (pathbuf), FALSE, FALSE))
	len =
	    GetPrivateProfileString (lpszSection, lpszEntry, lpszDefault,
	    lpszRetBuffer, cbRetBuffer, pathbuf);
      else
	{
	  CLEAR_ERROR ();
	  wSystemDSN = SYSTEMDSN_ONLY;
	  if (_iodbcadm_getinifile (pathbuf, sizeof (pathbuf), FALSE, FALSE))
	    len =
		GetPrivateProfileString (lpszSection, lpszEntry, lpszDefault,
		lpszRetBuffer, cbRetBuffer, pathbuf);
	}
      goto quit;
    };

  PUSH_ERROR (ODBC_ERROR_GENERAL_ERR);
  goto quit;

quit:
  wSystemDSN = USERDSN_ONLY;
  configMode = ODBC_BOTH_DSN;
  return len;
}
