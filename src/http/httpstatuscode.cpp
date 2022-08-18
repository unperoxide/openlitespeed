/*****************************************************************************
*    Open LiteSpeed is an open source HTTP server.                           *
*    Copyright (C) 2013 - 2022  LiteSpeed Technologies, Inc.                 *
*                                                                            *
*    This program is free software: you can redistribute it and/or modify    *
*    it under the terms of the GNU General Public License as published by    *
*    the Free Software Foundation, either version 3 of the License, or       *
*    (at your option) any later version.                                     *
*                                                                            *
*    This program is distributed in the hope that it will be useful,         *
*    but WITHOUT ANY WARRANTY; without even the implied warranty of          *
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the            *
*    GNU General Public License for more details.                            *
*                                                                            *
*    You should have received a copy of the GNU General Public License       *
*    along with this program. If not, see http://www.gnu.org/licenses/.      *
*****************************************************************************/
#include "httpstatuscode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lsr/ls_strtool.h>

LS_SINGLETON(HttpStatusCode);

int HttpStatusCode::s_codeToIndex[7] =
{
    0, SC_100, SC_200, SC_300, SC_400, SC_500, SC_END
};

HttpStatusCode::HttpStatusCode()
{
    int code = 0;
    m_aSC[code++] = new StatusCode(0, " 200 OK\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_100, " 100 Continue\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_101, " 101 Switching Protocols\r\n",
                                   NULL);
    m_aSC[code++] = new StatusCode(SC_102, " 102 Processing\r\n", NULL);

    m_aSC[code++] = new StatusCode(SC_200, " 200 OK\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_201, " 201 Created\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_202, " 202 Accepted\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_203,
                                   " 203 Non-Authoritative Information\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_204, " 204 No Content\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_205, " 205 Reset Content\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_206, " 206 Partial Content\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_207, " 207 Multi-Status\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_208, " 208 Already Reported\r\n", NULL);

    m_aSC[code++] = new StatusCode(SC_300, " 300 Multiple Choices\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_301, " 301 Moved Permanently\r\n",
                                   "The document has been permanently moved to <A HREF=\"%s\">here</A>.");
    m_aSC[code++] = new StatusCode(SC_302, " 302 Found\r\n",
                                   "The document has been temporarily moved to <A HREF=\"%s\">here</A>.");
    m_aSC[code++] = new StatusCode(SC_303, " 303 See Other\r\n",
                                   "The answer to your request is located <A HREF=\"%s\">here</A>.");
    m_aSC[code++] = new StatusCode(SC_304, " 304 Not Modified\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_305, " 305 Use Proxy\r\n",
                                   "The resource is only accessible through the proxy!");
    m_aSC[code++] = new StatusCode(SC_306, " 306 Switch Proxy\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_307, " 307 Temporary Redirect\r\n",
                                   "The document has been temporarily moved to <A HREF=\"%s\">here</A>.");
    m_aSC[code++] = new StatusCode(SC_308, " 308 Permanent Redirect\r\n",
                                   "The document has been permanently redirected.");

    m_aSC[code++] = new StatusCode(SC_400, " 400 Bad Request\r\n",
                                   "It is not a valid request!");
    m_aSC[code++] = new StatusCode(SC_401, " 401 Unauthorized\r\n",
                                   "Proper authorization is required to access this resource!");
    m_aSC[code++] = new StatusCode(SC_402, " 402 Payment Required\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_403, " 403 Forbidden\r\n",
                                   "Access to this resource on the server is denied!");
    m_aSC[code++] = new StatusCode(SC_404, " 404 Not Found\r\n",
                                   "The resource requested could not be found on this server!");
    m_aSC[code++] = new StatusCode(SC_405, " 405 Method Not Allowed\r\n",
                                   "This type request is not allowed!");
    m_aSC[code++] = new StatusCode(SC_406, " 406 Not Acceptable\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_407,
                                   " 407 Proxy Authentication Required\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_408, " 408 Request Time-out\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_409, " 409 Conflict\r\n",
                                   "The request could not be completed due to a conflict "
                                   "with the current state of the resource.");
    m_aSC[code++] = new StatusCode(SC_410, " 410 Gone\r\n",
                                   "The requested resource is no longer available at the server "
                                   "and no forwarding address is known.");
    m_aSC[code++] = new StatusCode(SC_411, " 411 Length Required\r\n",
                                   "Lenth of body must be present in the request header!");
    m_aSC[code++] = new StatusCode(SC_412, " 412 Precondition Failed\r\n",
                                   NULL);
    m_aSC[code++] = new StatusCode(SC_413, " 413 Request Entity Too Large\r\n",
                                   "The request body is over the maximum size allowed!");
    m_aSC[code++] = new StatusCode(SC_414, " 414 Request-URI Too Large\r\n",
                                   "The request URL is over the maximum size allowed!");
    m_aSC[code++] = new StatusCode(SC_415, " 415 Unsupported Media Type\r\n",
                                   "The media type is not supported by the server!");
    m_aSC[code++] = new StatusCode(SC_416,
                                   " 416 Requested range not satisfiable\r\n",
                                   "None of the range specified overlap the current extent of the selected resource.\n");
    m_aSC[code++] = new StatusCode(SC_417, " 417 Expectation Failed\r\n",
                                   NULL);
    m_aSC[code++] = new StatusCode(SC_418,
                                   " 418 reauthentication required\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_419,
                                   " 419 proxy reauthentication required\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_420, " 420 Policy Not Fulfilled\r\n",
                                   NULL);
    m_aSC[code++] = new StatusCode(SC_421, " 421 Bad Mapping\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_422, " 422 Unprocessable Entity\r\n",
                                   NULL);
    m_aSC[code++] = new StatusCode(SC_423, " 423 Locked\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_424, " 424 Failed Dependency\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_425, " 425 Too Early\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_426, " 426 Upgrade Required\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_427, " 427 \r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_428, " 428 Precondition Required\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_429, " 429 Too Many Requests\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_430, " 430 \r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_431, " 431 Request Header Fields Too Large\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_432, " 432 \r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_433, " 433 \r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_434, " 434 \r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_435, " 435 \r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_436, " 436 \r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_437, " 437 \r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_438, " 438 \r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_439, " 439 \r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_440, " 440 \r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_441, " 441 \r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_442, " 442 \r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_443, " 443 \r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_444, " 444 \r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_445, " 445 \r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_446, " 446 \r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_447, " 447 \r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_448, " 448 \r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_449, " 449 \r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_450, " 450 Website Unavailable\r\n", "The website you are trying to reach is unavailable due to security measures in place which restrict unauthorized access.");
    m_aSC[code++] = new StatusCode(SC_451, " 451 Unavailable For Legal Reasons\r\n", NULL);

    m_aSC[code++] = new StatusCode(SC_500, " 500 Internal Server Error\r\n",
                                   "An internal server error has occured.");
    m_aSC[code++] = new StatusCode(SC_501, " 501 Not Implemented\r\n",
                                   "The requested method is not implemented by the server.");
    m_aSC[code++] = new StatusCode(SC_502, " 502 Bad Gateway\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_503, " 503 Service Unavailable\r\n",
                                   "The server is temporarily busy, try again later!");
    m_aSC[code++] = new StatusCode(SC_504, " 504 Gateway Time-out\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_505,
                                   " 505 HTTP Version not supported\r\n",
                                   "Only HTTP/1.0, HTTP/1.1 is supported.");
    m_aSC[code++] = new StatusCode(SC_506, " 506 Loop Detected\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_507, " 507 Insufficient Storage\r\n",
                                   NULL);
    m_aSC[code++] = new StatusCode(SC_508, " 508 Insufficient Resource\r\n", NULL);
    m_aSC[code++] = new StatusCode(SC_509, " 509 Bandwidth Limit Exceeded\r\n",
                                   NULL);
    m_aSC[code++] = new StatusCode(SC_510, " 510 Not Extended\r\n", NULL);
};

HttpStatusCode::~HttpStatusCode()
{
    int code;
    for (code = 0; code < SC_END; ++code)
        delete m_aSC[code];
}

StatusCode::StatusCode(int code, const char *pStatus,
                       const char *message)
{
    memset(this, 0, sizeof(StatusCode));
    if (pStatus)
    {
        m_status = pStatus;
        status_size = strlen(pStatus);
        if (message)
        {
            char achBuf[4096];
            char *p = achBuf;
            char *pEnd = p + 4096;
            p += ls_snprintf(p, pEnd - p,
                             "<!DOCTYPE html>\n"
                             "<html style=\"height:100%%\">\n<head>\n"
                             "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, shrink-to-fit=no\">\n"
                             "<title>%s</title></head>\n"
                             "<body style=\"color: #444; margin:0;font: normal 14px/20px Arial, Helvetica, sans-serif; height:100%%; background-color: #fff;"
                             "\">\n"
                             "<div style=\"height:auto; min-height:100%%; \">"
                             "     <div style=\"text-align: center; width:800px; margin-left: -400px; position:absolute; top: 30%%; left:50%%;"
                             "\">\n"
                             "        <h1 style=\"margin:0; font-size:150px; line-height:150px; font-weight:bold;\">%c%c%c</h1>\n"
                             "<h2 style=\"margin-top:20px;font-size: 30px;\">%s</h2>\n"
                             "<p>%s</p>\n"
                             "</div></div>"
                             ,
                             pStatus, pStatus[1], pStatus[2], pStatus[3], &pStatus[5],
                             message);

            p += ls_snprintf(p, pEnd - p, "</body></html>\n");

            m_iBodySize = p - achBuf;
            m_pHeaderBody = (char *)malloc(m_iBodySize + 1);
            if (!m_pHeaderBody)
                m_iBodySize = 0;
            else
                memcpy(m_pHeaderBody, achBuf, m_iBodySize + 1);
        }
    }
}

StatusCode::~StatusCode()
{
    if (m_pHeaderBody)
        free(m_pHeaderBody);
}

int HttpStatusCode::codeToIndex(const char *code)
{
    char ch = *code++;
    if ((ch < '0') || (ch > '5'))
        return LS_FAIL;
    int offset = ch - '0';
    int index;
    ch = *code++;
    index = 10 * (ch - '0');
    ch = *code;
    if ((ch < '0') || (ch > '9'))
        return LS_FAIL;
    index += ch - '0';
    if (index < s_codeToIndex[offset + 1] - s_codeToIndex[offset])
        return s_codeToIndex[offset] + index;
    else
        return LS_FAIL;
}
