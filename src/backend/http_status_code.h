
#pragma once

enum class HTTPStatus {
  ERROR = -1, /* An error response from httpXxxx() */

  CONTINUE = 100,      /* Everything OK, keep going... */
  SWITCHING_PROTOCOLS, /* HTTP upgrade to TLS/SSL */

  OK = 200,          /* OPTIONS/GET/HEAD/POST/TRACE command was successful */
  CREATED,           /* PUT command was successful */
  ACCEPTED,          /* DELETE command was successful */
  NOT_AUTHORITATIVE, /* Information isn't authoritative */
  NO_CONTENT,        /* Successful command, no new data */
  RESET_CONTENT,     /* Content was reset/recreated */
  PARTIAL_CONTENT,   /* Only a partial file was recieved/sent */
  MULTI_STATUS,

  MULTIPLE_CHOICES = 300, /* Multiple files match request */
  MOVED_PERMANENTLY,      /* Document has moved permanently */
  MOVED_TEMPORARILY,      /* Document has moved temporarily */
  SEE_OTHER,              /* See this other link... */
  NOT_MODIFIED,           /* File not modified */
  USE_PROXY,              /* Must use a proxy to access this URI */

  BAD_REQUEST = 400,      /* Bad request */
  UNAUTHORIZED,           /* Unauthorized to access host */
  PAYMENT_REQUIRED,       /* Payment required */
  FORBIDDEN,              /* Forbidden to access this URI */
  NOT_FOUND,              /* URI was not found */
  METHOD_NOT_ALLOWED,     /* Method is not allowed */
  NOT_ACCEPTABLE,         /* Not Acceptable */
  PROXY_AUTHENTICATION,   /* Proxy Authentication is Required */
  REQUEST_TIMEOUT,        /* Request timed out */
  CONFLICT,               /* Request is self-conflicting */
  GONE,                   /* Server has gone away */
  LENGTH_REQUIRED,        /* A content length or encoding is required */
  PRECONDITION,           /* Precondition failed */
  REQUEST_TOO_LARGE,      /* Request entity too large */
  URI_TOO_LONG,           /* URI too long */
  UNSUPPORTED_MEDIATYPE,  /* The requested media type is unsupported */
  UPGRADE_REQUIRED = 426, /* Upgrade to SSL/TLS required */

  SERVER_ERROR = 500,  /* Internal server error */
  NOT_IMPLEMENTED,     /* Feature not implemented */
  BAD_GATEWAY,         /* Bad gateway */
  SERVICE_UNAVAILABLE, /* Service is unavailable */
  GATEWAY_TIMEOUT,     /* Gateway connection timed out */
  NOT_SUPPORTED        /* HTTP version not supported */
};
