- Install:
  - git clone git@github.com:deweerdt/h2o-exercise.git
  - git clone git@github.com:h2o/h2o.git
  - git clone git@github.com:deweerdt/h2get.git
  - Wireshark: https://www.wireshark.org/download.html
  - Firefox nightly: https://nightly.mozilla.org/
  - Chrome
  - nghttp2
  - curl
    - Linux: https://serversforhackers.com/video/curl-with-http2-support
    - Mac OS X: https://simonecarletti.com/blog/2016/01/http2-curl-macosx/
  - openssl
  - libuv (macosx only)

- Observe https://www.fastly.com
  - With Firefox
  - Find the pushes
  - With Chrome
  - Find the pushes
  - Use wireshark:
    - export SSLKEYLOGFILE=$HOME/sslkeylogs/keylog.log
    - /Applications/FirefoxNightly.app/Contents/MacOS/firefox &
    - wireshark


h2o server:
  - install and compile h2o:
    - cmake -DOPENSSL_ROOT_DIR=/usr/local/Cellar/openssl/1.0.2j/ .
    - make -j 4 && make install
  - Add a handler that sends a JS file
  Make that JS of variable size, denpending on an URL parameter, use
  h2o_req_t to find the path
  - Add a handler that sends a CSS file
  Make this of variable size
  - Add a handler that sends an HTML file
  Reference the js and css, multiple times
  - Add a 'Link: preload' header

- h2get:
  - the SETTINGS frame, try to obtain a GOAWAY
  - Send an wrong stream id -- use getp
  - Implement PING (https://www.google.com sends PING)
  - github.com/deweerdt/h2get -- no-decode-int
    https://tools.ietf.org/html/rfc7541#section-5.1
