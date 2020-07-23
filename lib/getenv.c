/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2020, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/

#include "curl_setup.h"

#include <curl/curl.h>
#include "curl_multibyte.h"
#include "curl_memory.h"

#include "memdebug.h"

static char *GetEnv(const char *variable)
{
#if defined(_WIN32_WCE) || defined(CURL_WINDOWS_APP)
  (void)variable;
  return NULL;
#elif defined(WIN32)
  /* This uses Windows API instead of C runtime getenv() to get the environment
     variable since some changes aren't always visible to the latter. #4774 */
  TCHAR *buf = NULL;
  TCHAR *tmp;
  DWORD bufsize;
  DWORD rc = 1;
  const DWORD max = 32768; /* max env var size from MSCRT source */
  TCHAR *tchar_envvar = curlx_convert_UTF8_to_tchar((char *)variable);

  for(;;) {
    tmp = realloc(buf, rc * sizeof(TCHAR));
    if(!tmp) {
      free(buf);
      curlx_unicodefree(tchar_envvar);
      return NULL;
    }

    buf = tmp;
    bufsize = rc;

    /* It's possible for rc to be 0 if the variable was found but empty.
       Since getenv doesn't make that distinction we ignore it as well. */
    rc = GetEnvironmentVariable(tchar_envvar, buf, bufsize);
    if(!rc || rc == bufsize || rc > max) {
      free(buf);
      curlx_unicodefree(tchar_envvar);
      return NULL;
    }

    /* if rc < bufsize then rc is bytes written not including null */
    if(rc < bufsize) {
      char *path = curlx_convert_tchar_to_UTF8(buf);
      curlx_unicodefree(buf);
      curlx_unicodefree(tchar_envvar);
      return path;
    }

    /* else rc is bytes needed, try again */
  }
#else
  char *env = getenv(variable);
  return (env && env[0])?strdup(env):NULL;
#endif
}

char *curl_getenv(const char *v)
{
  return GetEnv(v);
}
