#include "http.h"

#include "os/c.h"
#include "util/assert.h"
#include "util/int.h"
#include "util/new.h"
#include "util/string-view.h"
#include "util/string.h"
#include "util/vector.h"

// curl/curl.h
#if MSVC
#    define CURL_EXTERN __declspec(dllimport)
#else
#    define CURL_EXTERN
#endif
#define CURLOPTTYPE_LONG          0
#define CURLOPTTYPE_OBJECTPOINT   10000
#define CURLOPTTYPE_STRINGPOINT   10000
#define CURLOPTTYPE_FUNCTIONPOINT 20000
#define CURLOPTTYPE_OFF_T         30000
typedef void CURL;
enum CURLcode {
    CURLE_OK = 0,
};
enum CURLoption {
    CURLOPT_WRITEDATA = CURLOPTTYPE_OBJECTPOINT + 1,
    CURLOPT_URL = CURLOPTTYPE_STRINGPOINT + 2,
    CURLOPT_READDATA = CURLOPTTYPE_OBJECTPOINT + 9,
    CURLOPT_ERRORBUFFER = CURLOPTTYPE_OBJECTPOINT + 10,
    CURLOPT_WRITEFUNCTION = CURLOPTTYPE_FUNCTIONPOINT + 11,
    CURLOPT_READFUNCTION = CURLOPTTYPE_FUNCTIONPOINT + 12,
    CURLOPT_USERAGENT = CURLOPTTYPE_STRINGPOINT + 18,
    CURLOPT_COOKIE = CURLOPTTYPE_STRINGPOINT + 22,
    CURLOPT_HTTPHEADER = CURLOPTTYPE_OBJECTPOINT + 23,
    CURLOPT_VERBOSE = CURLOPTTYPE_LONG + 41,
    CURLOPT_UPLOAD = CURLOPTTYPE_LONG + 46,
    CURLOPT_POST = CURLOPTTYPE_LONG + 47,
    CURLOPT_HTTPGET = CURLOPTTYPE_LONG + 80,
    CURLOPT_NOSIGNAL = CURLOPTTYPE_LONG + 99,
    CURLOPT_INFILESIZE_LARGE = CURLOPTTYPE_OFF_T + 115,
};
struct CURLlist {
    const char* data;
    struct CURLlist* next;
};

extern "C" {
// curl/easy.h
CURL_EXTERN CURL*
curl_easy_init(void) noexcept;

CURL_EXTERN enum CURLcode
curl_easy_setopt(CURL*, enum CURLoption, ...) noexcept;

CURL_EXTERN enum CURLcode
curl_easy_perform(CURL*) noexcept;

CURL_EXTERN void
curl_easy_cleanup(CURL*) noexcept;
}

static Size
writeBody(char* output, Size size, Size nitems, StringView* body) noexcept {
    const void* data = body->data;
    Size written = body->size;
    if (written > size * nitems) {
        written = size * nitems;
    }

    body->data += written;
    body->size -= written;

    memcpy(output, data, written);

    return written;
}

static Size
readBody(char* input, Size size, Size nitems, String* body) noexcept {
    Size read = size * nitems;

    StringView in(input, read);
    (*body) << in;

    return read;
}

struct Http {
    CURL* curl;
    char errmsg[256];
    String buf;
    String cookie;
    Vector<CURLlist> list;
};

Http*
httpMake(bool verbose) noexcept {
    enum CURLcode err;
    (void)err;

    Http* self = xmalloc(Http, 1);

    self->curl = curl_easy_init();
    assert_(self->curl);

    err = curl_easy_setopt(self->curl, CURLOPT_ERRORBUFFER, self->errmsg);
    assert_(err == CURLE_OK);

    err = curl_easy_setopt(self->curl, CURLOPT_NOSIGNAL, 1L);
    assert_(err == CURLE_OK);

    err = curl_easy_setopt(self->curl, CURLOPT_USERAGENT, "tsunagari/0.1");
    assert_(err == CURLE_OK);

    if (verbose) {
        err = curl_easy_setopt(self->curl, CURLOPT_VERBOSE, 1L);
        assert_(err == CURLE_OK);
    }

    return self;
}

void
httpDestroy(Http* self) noexcept {
    curl_easy_cleanup(self->curl);
    free(self);
}

void
httpSetCookie(Http* self, String cookie) noexcept {
    enum CURLcode err;
    (void)err;

    self->cookie = static_cast<String&&>(cookie);
    self->cookie.null();

    err = curl_easy_setopt(self->curl, CURLOPT_COOKIE, self->cookie.data);
    assert_(err == CURLE_OK);
}

StringView
httpGet(Http* self, String* response, StringView url, Header* headers,
        Size headersSize) noexcept {
    self->buf.clear();
    if (self->buf.capacity < 1024) {
        self->buf.reserve(1024);
    }

    self->buf << url << static_cast<char>(0);

    self->list.clear();
    if (self->list.capacity < headersSize) {
        self->list.reserve(headersSize);
    }

    for (Size i = 0; i < headersSize; i++) {
        Header* header = headers + i;

        CURLlist item;
        // Pun item.data and store an offset.
        item.data = reinterpret_cast<char*>(self->buf.size);
        item.next = 0;
        self->list.push(item);
        self->buf << header->name << ": " << header->value
                  << static_cast<char>(0);
    }

    for (CURLlist* item = self->list.begin();
         item != self->list.end() && item + 1 != self->list.end();
         ++item) {
        item[0].next = &item[1];
    }

    // Now that buf has a stable base pointer, use it.
    char* url_ = self->buf.data;
    for (CURLlist* item = self->list.begin(); item != self->list.end();
         ++item) {
        // It's actually the other way around... item has an offset and buf has
        // the base pointer.
        item->data += reinterpret_cast<Size>(self->buf.data);
    }

    enum CURLcode err;

    err = curl_easy_setopt(self->curl, CURLOPT_HTTPGET, 1L);
    assert_(err == CURLE_OK);
    err = curl_easy_setopt(self->curl, CURLOPT_URL, url_);
    assert_(err == CURLE_OK);
    err = curl_easy_setopt(self->curl, CURLOPT_HTTPHEADER, self->list.data);
    assert_(err == CURLE_OK);
    err = curl_easy_setopt(self->curl, CURLOPT_WRITEFUNCTION, readBody);
    assert_(err == CURLE_OK);
    err = curl_easy_setopt(self->curl, CURLOPT_WRITEDATA, response);
    assert_(err == CURLE_OK);
    err = curl_easy_setopt(self->curl, CURLOPT_READFUNCTION, 0);
    assert_(err == CURLE_OK);
    err = curl_easy_setopt(self->curl, CURLOPT_READDATA, 0);
    assert_(err == CURLE_OK);
    err = curl_easy_setopt(self->curl, CURLOPT_INFILESIZE_LARGE, 0);
    assert_(err == CURLE_OK);

    err = curl_easy_perform(self->curl);
    if (err) {
        return StringView(self->errmsg, strlen(self->errmsg));
    }

    return StringView();
}

StringView
httpForm(Http* self, String* response, StringView url, Header* headers,
         Size headersSize, FormData* data, Size dataSize) noexcept {
    self->buf.clear();
    if (self->buf.capacity < 1024) {
        self->buf.reserve(1024);
    }

    self->buf << url << static_cast<char>(0);

    self->list.clear();
    if (self->list.capacity < 1 + headersSize) {
        self->list.reserve(1 + headersSize);
    }

    CURLlist contentType;
    // Pun and store offset.
    contentType.data = reinterpret_cast<char*>(self->buf.size);
    contentType.next = 0;
    self->list.push(contentType);
    self->buf << "Content-Type: multipart/form-data; boundary=BOUNDARY"
              << static_cast<char>(0);

    for (Size i = 0; i < headersSize; i++) {
        Header* header = headers + i;

        CURLlist item;
        // Pun item.data and store an offset.
        item.data = reinterpret_cast<char*>(self->buf.size);
        item.next = 0;
        self->list.push(item);
        self->buf << header->name << ": " << header->value
                  << static_cast<char>(0);
    }

    for (CURLlist* item = self->list.begin(); item + 1 != self->list.end();
         ++item) {
        item[0].next = &item[1];
    }

    Size bodyStart = self->buf.size;
    for (Size i = 0; i < dataSize; i++) {
        FormData* entity = data + i;

        self->buf << "--BOUNDARY\r\nContent-Disposition: form-data; name=\""
                  << entity->name << "\"\r\n\r\n"
                  << entity->value << "\r\n";
    }
    self->buf << "--BOUNDARY--\r\n";

    // Now that buf has a stable base pointer, use it.
    char* url_ = self->buf.data;
    for (CURLlist* item = self->list.begin(); item != self->list.end();
         ++item) {
        // It's actually the other way around... item has an offset and buf has
        // the base pointer.
        item->data += reinterpret_cast<Size>(self->buf.data);
    }
    StringView body = self->buf.view().substr(bodyStart);

    enum CURLcode err;

    err = curl_easy_setopt(self->curl, CURLOPT_POST, 1L);
    assert_(err == CURLE_OK);
    err = curl_easy_setopt(self->curl, CURLOPT_URL, url_);
    assert_(err == CURLE_OK);
    err = curl_easy_setopt(self->curl, CURLOPT_HTTPHEADER, self->list.data);
    assert_(err == CURLE_OK);
    err = curl_easy_setopt(self->curl, CURLOPT_WRITEFUNCTION, readBody);
    assert_(err == CURLE_OK);
    err = curl_easy_setopt(self->curl, CURLOPT_WRITEDATA, response);
    assert_(err == CURLE_OK);
    err = curl_easy_setopt(self->curl, CURLOPT_READFUNCTION, writeBody);
    assert_(err == CURLE_OK);
    err = curl_easy_setopt(self->curl, CURLOPT_READDATA, &body);
    assert_(err == CURLE_OK);
    err = curl_easy_setopt(self->curl, CURLOPT_INFILESIZE_LARGE, body.size);
    assert_(err == CURLE_OK);

    err = curl_easy_perform(self->curl);
    if (err) {
        return StringView(self->errmsg, strlen(self->errmsg));
    }

    return StringView();
}
