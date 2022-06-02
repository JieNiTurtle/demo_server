#include "../../include/http/httpcontext.h"

namespace demo
{
    namespace http
    {

        bool HttpContext::processRequestLine(const char *begin, const char *end)
        {
            bool succeed = false;
            const char *start = begin;
            const char *space = std::find(start, end, ' ');
            if (space != end && request.setMethod(start, space))
            {
                start = space + 1;
                space = std::find(start, end, ' ');
                if (space != end)
                {
                    const char *question = std::find(start, space, '?');
                    if (question != space)
                    {
                        request.setPath(start, question);
                        request.setQuery(question + 1, space);
                    }
                    else
                    {
                        request.setPath(start, space);
                    }
                    start = space + 1;
                    succeed = end - start == 8 && std::equal(start, end - 1, "HTTP/1.");
                    if (succeed)
                    {
                        if (*(end - 1) == '1')
                        {
                            request.setVersion(HttpRequest::HTTP11);
                        }
                        else if (*(end - 1) == '0')
                        {
                            request.setVersion(HttpRequest::HTTP10);
                        }
                        else
                        {
                            succeed = false;
                        }
                    }
                }
            }
            return succeed;
        }

        bool HttpContext::parseRequest(Buffer *buf)
        {
            bool ok = true;
            bool hasMore = true;
            while (hasMore)
            {
                if (state == EXPECT_REQUEST_LINE)
                {
                    const char *crlf = buf->findCRLF();
                    if (crlf)
                    {
                        ok = processRequestLine(buf->peek(), crlf);
                        if (ok)
                        {
                            buf->retrieveUntil(crlf + 2);
                            state = EXPECT_HEADERS;
                        }
                        else
                        {
                            hasMore = false;
                        }
                    }
                    else
                    {
                        hasMore = false;
                    }
                }
                else if (state == EXPECT_HEADERS)
                {
                    const char *crlf = buf->findCRLF();
                    if (crlf)
                    {
                        const char *colon = std::find(buf->peek(), crlf, ':');
                        if (colon != crlf)
                        {
                            request.addHeader(buf->peek(), colon, crlf);
                        }
                        else
                        {
                            state = GOT_ALL;
                            hasMore = false;
                        }
                        buf->retrieveUntil(crlf + 2);
                    }
                    else
                    {
                        hasMore = false;
                    }
                }
            }
            return ok;
        }
    }
}