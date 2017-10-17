<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [Needed software](#needed-software)
- [The observation track](#the-observation-track)
  - [With Chrome](#with-chrome)
  - [With Firefox](#with-firefox)
- [The low-level client track](#the-low-level-client-track)
  - [Establishing a connection](#establishing-a-connection)
  - [GOAWAY frames](#goaway-frames)
  - [PING frames](#ping-frames)
  - [HPACK](#hpack)
- [The server track](#the-server-track)
  - [H2O frame handling](#h2o-frame-handling)
  - [Using H2O as a library](#using-h2o-as-a-library)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Needed software

  - git clone git@github.com:deweerdt/h2o-exercise.git
  - git clone git@github.com:h2o/h2o.git
  - git clone git@github.com:deweerdt/h2get.git
  - Wireshark: https://www.wireshark.org/download.html
  - Chrome
  - nghttp2
  - curl
    - Linux: https://serversforhackers.com/video/curl-with-http2-support
    - Mac OS X: https://simonecarletti.com/blog/2016/01/http2-curl-macosx/
  - openssl
  - libuv (macosx only)

# The observation track

## With Chrome

Using chrome://net-internals#http2 . Note that you can use "Flush
Sockets" in the upper right corner to force Chrome to re-establish
an HTTP/2 connection.

- Observe connection coalescing:
  - https://www.vox.com
  - Verify that they share the same IP:
    ```
    dig cdn.vox-cdn.com
    dig fonts.voxmedia.com
    ```
  - Check the SAN:
    `openssl s_client -connect fonts.voxmedia.com:443 2>&1 | openssl x509 -text | grep -A1 'Subject Alternative Name'`
- Observer pushes
  - https://seekingalpha.com/
  - find `HTTP2_SESSION_RECV_PUSH_PROMISE`
  - observe how H2O replicated some of the headers for the request that originated the push

## With Firefox
- Use wireshark (https://developer.mozilla.org/en-US/docs/Mozilla/Projects/NSS/Key_Log_Format):
  ```
  $ mkdir $HOME/sslkeylogs/
  $ export SSLKEYLOGFILE=$HOME/sslkeylogs/keylog.log
  $ /Applications/Firefox.app/Contents/MacOS/firefox &
  $ wireshark
  ```
- Observe how Firefox sends PRIORITY frames and then attaches requests to them

# The low-level client track

Build h2get:
- `cmake -DOPENSSL_ROOT_DIR=/usr/local/Cellar/openssl@1.1/1.1.0f .`

`h2get` embeds a Ruby interpreter, it can be used as following: `./h2get script.rb`

## Establishing a connection

- Dump the different settings that different servers send:
  - www.fastly.com
  - www.google.com
  - www.facebook.com

## GOAWAY frames

From RFC 7540, 5.4.1. Connection Error Handling
(https://tools.ietf.org/html/rfc7540#section-5.4.1)

> An endpoint that encounters a connection error SHOULD first send a
> GOAWAY frame (Section 6.8) with the stream identifier of the last
> stream that it successfully received from its peer

- Send invalid values in the SETTINGS frame in order to obtain a
response from the server (search for `PROTOCOL_ERROR` in the RFC)
- Send an invalid stream id -- use `send_headers`

## PING frames

Have a client sending send PING frames (https://www.google.com or
https://www.fastly.com respond to PING)

## HPACK

Implement `decode_int` https://tools.ietf.org/html/rfc7541#section-5.1

Checkout the `no-decode-int` branch, use `./test_decode_int` to check the implementation.

# The server track

## H2O frame handling

H2O core frame handling function is
`lib/http2/connection.c::expect_default`. This function
dispatches the received frame depending on its type. Apply
`0001-Output-priorities.patch` in order to have priority information
about the requests output to stderr.

Following the same model, instrument `handle_window_update_frame`
in order to understand how browsers send `WINDOW_UPDATE` frames.

`www/` has a directory that can be used as a root directory to serve
different type of contents.

## Using H2O as a library

This assumes that you've built and installed h2o. This repository
comes with a small C server that uses libh2o that we will customize.
- install and compile h2o:
  - `cmake -DOPENSSL_ROOT_DIR=/usr/local/Cellar/openssl@1.1/1.1.0f/ .`
  - make -j 4 && make install
- Build the sample server in this directory:
  - `make`
- Add a handler that sends a JS file
  Make that JS of variable size, denpending on an URL parameter, use
  `h2o_req_t` to find the path of the request .
- Add a handler that sends a CSS file
  Make this of variable size
- Add a handler that sends an HTML file
  Reference the js and css, multiple times
- Add a 'Link: preload' header
  - See the upcoming `critical` keyword: https://github.com/h2o/h2o/pull/1436

   
