/*
 *  SQLInstallerError.c
 *
 *  $Id: SQLInstallerError.c,v 1.1.1.1 2002/04/08 22:48:11 miner Exp $
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

#include "iodbc_error.h"

LPSTR errortable[] = {
  "",
  "General installer error",
  "Invalid buffer length",
  "Invalid window handle",
  "Invalid string parameter",
  "Invalid type of request",
  "Component not found",
  "Invalid name parameter",
  "Invalid keyword-value pairs",
  "Invalid DSN",
  "Invalid .INF file",
  "Request failed",
  "Invalid install path.",
  "Could not load the driver or translator setup library",
  "Invalid parameter sequence",
  "Invalid log file name.",
  "Operation canceled on user request",
  "Could not increment or decrement the component usage count",
  "Creation of the DSN failed",
  "Error during writing system information",
  "Deletion of the DSN failed",
  "Out of memory",
  "Output string truncated due to a buffer not large enough",
};


RETCODE INSTAPI
SQLInstallerError (WORD iError, DWORD *pfErrorCode, LPSTR lpszErrorMsg,
    WORD cbErrorMsgMax, WORD * pcbErrorMsg)
{
  LPSTR message;
  RETCODE retcode = SQL_ERROR;

  /* Check if the index is valid to retrieve an error */
  if ((iError - 1) > numerrors)
    {
      retcode = SQL_NO_DATA;
      goto quit;
    }

  if (!lpszErrorMsg || !cbErrorMsgMax)
    goto quit;

  lpszErrorMsg[cbErrorMsgMax - 1] = 0;

  /* Copy the message error */
  message = (errormsg[iError - 1]) ?
	errormsg[iError - 1] : errortable[ierror[iError - 1]];

  if (STRLEN (message) >= cbErrorMsgMax - 1)
    {
      STRNCPY (lpszErrorMsg, message, cbErrorMsgMax - 1);
      retcode = SQL_SUCCESS_WITH_INFO;
      goto quit;
    }
  else
    STRCPY (lpszErrorMsg, message);

  if (pfErrorCode)
    *pfErrorCode = ierror[iError - 1];
  if (pcbErrorMsg)
    *pcbErrorMsg = STRLEN (lpszErrorMsg);
  retcode = SQL_SUCCESS;

quit:
  return retcode;
}
