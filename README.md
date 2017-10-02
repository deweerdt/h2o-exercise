- Install:
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

- Observe https://seekingalpha.com/
  - With Firefox
  - Find the pushes
  - With Chrome
  - Find the pushes
  - Use wireshark (https://developer.mozilla.org/en-US/docs/Mozilla/Projects/NSS/Key_Log_Format):
    - export SSLKEYLOGFILE=$HOME/sslkeylogs/keylog.log
    - /Applications/Firefox.app/Contents/MacOS/firefox &
    - wireshark


h2o server:
  - install and compile h2o:
    - `cmake -DOPENSSL_ROOT_DIR=/usr/local/Cellar/openssl@1.1/1.1.0f/ .`
    - make -j 4 && make install
  - Add a handler that sends a JS file
  Make that JS of variable size, denpending on an URL parameter, use
  `h2o_req_t` to find the path of the request .
  - Add a handler that sends a CSS file
  Make this of variable size
  - Add a handler that sends an HTML file
  Reference the js and css, multiple times
  - Add a 'Link: preload' header
  - See the upcoming `critical` keyword: https://github.com/h2o/h2o/pull/1436

- h2get:
  - `cmake -DOPENSSL_ROOT_DIR=/usr/local/Cellar/openssl@1.1/1.1.0f .`
  - the SETTINGS frame, try to obtain a GOAWAY
  - Send an wrong stream id -- use `send_headers`
  - Implement PING (https://www.google.com or https://www.fastly.com respond to PING)
  - Implement decode_int https://tools.ietf.org/html/rfc7541#section-5.1
    github.com/deweerdt/h2get -- no-decode-int
    
