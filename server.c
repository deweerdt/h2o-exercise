#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <h2o.h>


static int on_req_big(h2o_handler_t *self, h2o_req_t *req)
{
  static h2o_generator_t generator = {};
#define BIGLEN (100*1024)
  static char buf[BIGLEN];
  static h2o_iovec_t body = {buf, BIGLEN};
  if (!h2o_memis(req->method.base, req->method.len, H2O_STRLIT("GET"))) {
    return -1;
  }
  memset(buf, 'a', sizeof(buf));
  req->res.status = 200;
  req->res.reason = "OK";
  h2o_add_header(&req->pool, &req->res.headers, H2O_TOKEN_CONTENT_TYPE, H2O_STRLIT("text/plain"));
  h2o_start_response(req, &generator);
  h2o_send(req, &body, 1, 1);
  return 0;
}

static int on_req(h2o_handler_t *self, h2o_req_t *req)
{
  static h2o_generator_t generator = {};
  static h2o_iovec_t body = {H2O_STRLIT("hello world\n")};
  if (!h2o_memis(req->method.base, req->method.len, H2O_STRLIT("GET"))) {
    return -1;
  }
  req->res.status = 200;
  req->res.reason = "OK";
  h2o_add_header(&req->pool, &req->res.headers, H2O_TOKEN_CONTENT_TYPE, H2O_STRLIT("text/plain"));
  h2o_start_response(req, &generator);
  h2o_send(req, &body, 1, 1);
  return 0;
}

static h2o_globalconf_t config;
static h2o_accept_ctx_t accept_ctx;
static h2o_context_t ctx;

#if H2O_USE_LIBUV

static void on_accept(uv_stream_t *listener, int status)
{
  uv_tcp_t *conn;
  h2o_socket_t *sock;

  if (status != 0)
    return;

  conn = h2o_mem_alloc(sizeof(*conn));
  uv_tcp_init(listener->loop, conn);

  if (uv_accept(listener, (uv_stream_t *)conn) != 0) {
    uv_close((uv_handle_t *)conn, (uv_close_cb)free);
    return;
  }

  sock = h2o_uv_socket_create((uv_stream_t *)conn, (uv_close_cb)free);
  h2o_accept(&accept_ctx, sock);
}

static int create_listener(unsigned short port)
{
  static uv_tcp_t listener;
  struct sockaddr_in addr;
  int r;

  uv_tcp_init(ctx.loop, &listener);
  uv_ip4_addr("127.0.0.1", port, &addr);
  if ((r = uv_tcp_bind(&listener, (struct sockaddr *)&addr, 0)) != 0) {
    fprintf(stderr, "uv_tcp_bind:%s\n", uv_strerror(r));
    goto Error;
  }
  if ((r = uv_listen((uv_stream_t *)&listener, 128, on_accept)) != 0) {
    fprintf(stderr, "uv_listen:%s\n", uv_strerror(r));
    goto Error;
  }

  return 0;
Error:
  uv_close((uv_handle_t *)&listener, NULL);
  return r;
}


#else

static void on_accept(h2o_socket_t *listener, const char *err)
{
  h2o_socket_t *sock;

  if (err != NULL) {
    return;
  }

  if ((sock = h2o_evloop_socket_accept(listener)) == NULL)
    return;
  h2o_accept(&accept_ctx, sock);
}

static int create_listener(unsigned short port)
{
  struct sockaddr_in addr;
  int fd, reuseaddr_flag = 1;
  h2o_socket_t *sock;

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(0x7f000001);
  addr.sin_port = htons(port);

  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ||
      setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_flag, sizeof(reuseaddr_flag)) != 0 ||
      bind(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0 || listen(fd, SOMAXCONN) != 0) {
    return -1;
  }

  sock = h2o_evloop_socket_create(ctx.loop, fd, H2O_SOCKET_FLAG_DONT_READ);
  h2o_socket_read_start(sock, on_accept);

  return 0;
}

#endif

static int setup_ssl(const char *ssl_dir, const char **err)
{
    char cert_file[PATH_MAX];
    char key_file[PATH_MAX];

    SSL_load_error_strings();
    SSL_library_init();
    OpenSSL_add_all_algorithms();

    accept_ctx.ssl_ctx = SSL_CTX_new(SSLv23_server_method());
    SSL_CTX_set_options(accept_ctx.ssl_ctx, SSL_OP_NO_SSLv2);

#define CERT_FILE "server.crt"
#define KEY_FILE "server.key"
    sprintf(cert_file, "%s/%s", ssl_dir, CERT_FILE);
    sprintf(key_file, "%s/%s", ssl_dir, KEY_FILE);
    /* load certificate and private key */
    if (SSL_CTX_use_certificate_file(accept_ctx.ssl_ctx, cert_file, SSL_FILETYPE_PEM) != 1) {
        *err = "an error occurred while trying to load server certificate file " CERT_FILE "\n";
        return -1;
    }
    if (SSL_CTX_use_PrivateKey_file(accept_ctx.ssl_ctx, key_file, SSL_FILETYPE_PEM) != 1) {
        *err = "an error occurred while trying to load private key file " KEY_FILE "\n";
        return -1;
    }

/* setup protocol negotiation methods */
#if H2O_USE_NPN
    h2o_ssl_register_npn_protocols(accept_ctx.ssl_ctx, h2o_http2_npn_protocols);
#endif
#if H2O_USE_ALPN
    h2o_ssl_register_alpn_protocols(accept_ctx.ssl_ctx, h2o_http2_alpn_protocols);
#endif

    return 0;
}

int main(int argc, char **argv)
{
  unsigned short port = 8080;
  const char *ssl_dir = NULL;
  h2o_hostconf_t *hostconf;

  if (argc > 1) {
    port = atoi(argv[1]);
  }
  if (argc > 2) {
    ssl_dir = argv[2];
  }

  signal(SIGPIPE, SIG_IGN);

  h2o_config_init(&config);
  hostconf = h2o_config_register_host(&config, h2o_iovec_init(H2O_STRLIT("default")), 65535);

  h2o_pathconf_t *pathconf_big = h2o_config_register_path(hostconf, "/big", 0);
  h2o_handler_t *handler_big = h2o_create_handler(pathconf_big, sizeof(*handler_big));
  handler_big->on_req = on_req_big;

  h2o_pathconf_t *pathconf = h2o_config_register_path(hostconf, "/", 0);
  h2o_handler_t *handler = h2o_create_handler(pathconf, sizeof(*handler));
  handler->on_req = on_req;

#if H2O_USE_LIBUV
  uv_loop_t loop;
  uv_loop_init(&loop);
  h2o_context_init(&ctx, &loop, &config);
#else
  h2o_context_init(&ctx, h2o_evloop_create(), &config);
#endif

  h2o_access_log_register(pathconf, h2o_access_log_open_handle("/dev/stdout", NULL));

  accept_ctx.ctx = &ctx;
  accept_ctx.hosts = config.hosts;

  if (ssl_dir) {
    const char *err = NULL;
    if (setup_ssl(ssl_dir, &err) != 0) {
      fprintf(stderr, "Failed to setup SSL: %s\n", err);
      exit(1);
    }
  }
  if (create_listener(port) != 0) {
    fprintf(stderr, "failed to listen to 127.0.0.1:%d:%s\n", port, strerror(errno));
    exit(1);
  }

#if H2O_USE_LIBUV
  uv_run(ctx.loop, UV_RUN_DEFAULT);
#else
  while (h2o_evloop_run(ctx.loop, INT32_MAX) == 0)
    ;
#endif
}
