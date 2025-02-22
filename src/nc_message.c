/*
 * twemproxy - A fast and lightweight proxy for memcached protocol.
 * Copyright (C) 2011 Twitter, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <aws/nc_aws.h>
#include <math.h>
#include <nc_core.h>
#include <nc_server.h>
#include <oscm/nc_oscm.h>
#include <proto/nc_proto.h>
#include <redis/nc_redis.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/uio.h>

#if (IOV_MAX > 128)
//#define NC_IOV_MAX 128
#define NC_IOV_MAX 1024
#else
#define NC_IOV_MAX IOV_MAX
#endif

/*
 *            nc_message.[ch]
 *         message (struct msg)
 *            +        +            .
 *            |        |            .
 *            /        \            .
 *         Request    Response      .../ nc_mbuf.[ch]  (mesage buffers)
 *      nc_request.c  nc_response.c .../ nc_memcache.c; nc_redis.c (message parser)
 *
 * Messages in nutcracker are manipulated by a chain of processing handlers,
 * where each handler is responsible for taking the input and producing an
 * output for the next handler in the chain. This mechanism of processing
 * loosely conforms to the standard chain-of-responsibility design pattern
 *
 * At the high level, each handler takes in a message: request or response
 * and produces the message for the next handler in the chain. The input
 * for a handler is either a request or response, but never both and
 * similarly the output of an handler is either a request or response or
 * nothing.
 *
 * Each handler itself is composed of two processing units:
 *
 * 1). filter: manipulates output produced by the handler, usually based
 *     on a policy. If needed, multiple filters can be hooked into each
 *     location.
 * 2). forwarder: chooses one of the backend servers to send the request
 *     to, usually based on the configured distribution and key hasher.
 *
 * Handlers are registered either with Client or Server or Proxy
 * connections. A Proxy connection only has a read handler as it is only
 * responsible for accepting new connections from client. Read handler
 * (conn_recv_t) registered with client is responsible for reading requests,
 * while that registered with server is responsible for reading responses.
 * Write handler (conn_send_t) registered with client is responsible for
 * writing response, while that registered with server is responsible for
 * writing requests.
 *
 * Note that in the above discussion, the terminology send is used
 * synonymously with write or OUT event. Similarly recv is used synonymously
 * with read or IN event
 *
 *             Client+             Proxy           Server+
 *                              (nutcracker)
 *                                   .
 *       msg_recv {read event}       .       msg_recv {read event}
 *         +                         .                         +
 *         |                         .                         |
 *         \                         .                         /
 *         req_recv_next             .             rsp_recv_next
 *           +                       .                       +
 *           |                       .                       |       Rsp
 *           req_recv_done           .           rsp_recv_done      <===
 *             +                     .                     +
 *             |                     .                     |
 *    Req      \                     .                     /
 *    ===>     req_filter*           .           *rsp_filter
 *               +                   .                   +
 *               |                   .                   |
 *               \                   .                   /
 *               req_forward-//  (a) . (c)  \\-rsp_forward
 *                                   .
 *                                   .
 *       msg_send {write event}      .      msg_send {write event}
 *         +                         .                         +
 *         |                         .                         |
 *    Rsp' \                         .                         /     Req'
 *   <===  rsp_send_next             .             req_send_next     ===>
 *           +                       .                       +
 *           |                       .                       |
 *           \                       .                       /
 *           rsp_send_done-//    (d) . (b)    //-req_send_done
 *
 *
 * (a) -> (b) -> (c) -> (d) is the normal flow of transaction consisting
 * of a single request response, where (a) and (b) handle request from
 * client, while (c) and (d) handle the corresponding response from the
 * server.
 */

static uint64_t msg_id;          /* message id counter */
static uint64_t frag_id;         /* fragment id counter */
static uint32_t nfree_msgq;      /* # free msg q */
static struct msg_tqh free_msgq; /* free msg q */
static struct rbtree tmo_rbt;    /* timeout rbtree */
static struct rbnode tmo_rbs;    /* timeout rbtree sentinel */

/* Record related variables and functions */

static int record_buffer_size = 2000;
static int record_buffer_wbeg = 0;           /* will be written to file from here */
static int record_buffer_wend = 0;           /* will be written to file until here */
static int record_buffer_curr = 0;           /* new records will be put here */
static struct request_record *record_buffer; /* sets of records*/

static pthread_t record_thread;
static int record_alive = 1;
struct record_ctx *rctx;

void record_init() {
    /* create record directory */
    /* new: while writing to file / mid: copied from new to mid / old: already moved to MasterMaster */
    loga("[record_init] mkdir mac_records");
    char cmd1[] = "mkdir -p /tmp/mac_records/new";
    char cmd2[] = "mkdir -p /tmp/mac_records/mid";
    char cmd3[] = "mkdir -p /tmp/mac_records/old";
    system(cmd1);
    system(cmd2);
    system(cmd3);

    record_buffer = (struct request_record *)nc_alloc(sizeof(struct request_record) * record_buffer_size);
    for (int i = 0; i < record_buffer_size; i++)
        record_buffer[i].key = (char *)nc_alloc(sizeof(char) * (MACARON_MAX_KEY_LEN + 1));

    rctx = (struct record_ctx *)nc_alloc(sizeof(struct record_ctx));
    rctx->alive = &record_alive;
    rctx->buffer = &record_buffer;
    rctx->wbeg = &record_buffer_wbeg;
    rctx->wend = &record_buffer_wend;
    rctx->cend = &record_buffer_curr;

    pthread_create(&record_thread, NULL, record_worker, NULL);
}

void record_deinit() {
    loga("[record_deinit] deinitialize record related stuffs...");
    pthread_join(record_thread, NULL);
    for (int i = 0; i < record_buffer_size; i++)
        nc_free((*rctx->buffer)[i].key);
    nc_free(*rctx->buffer);
    nc_free(rctx);

    for (int i = 0; i < record_buffer_size; i++)
        nc_free(record_buffer[i].key);
    nc_free(record_buffer);
}

float get_timediff_sec(struct timeval t0, struct timeval t1) {
    return t1.tv_sec - t0.tv_sec;
}

uint32_t get_elapsed_time(struct msg *msg) {
    return (uint32_t)(nc_usec_now() - msg->start_ts);
}

void *record_worker(void *arg) {
    struct record_ctx *ctx = rctx;
    int record_interval_sec = 60;  // 60 second
    struct timeval record_time, curr_time;
    gettimeofday(&record_time, NULL);

    int record_idx = 1;
    char *ofname = nc_alloc(sizeof(char) * 100);
    char *linestr = nc_alloc(sizeof(char) * 300);
    while (*(ctx->alive) == 1) {
        gettimeofday(&curr_time, NULL);
        int timediff_sec = (int)get_timediff_sec(record_time, curr_time);
        if (timediff_sec > record_interval_sec) {  // record requests to the file
            loga("[record_worker] Start writing proxy access log");
            *(ctx->wend) = *(ctx->cend);
            if (*(ctx->wbeg) == *(ctx->wend)) {  // nothing to write
            } else {
                sleep(1);  // XXX: wait for the last record to be fully written
                snprintf(ofname, 100, "/tmp/mac_records/new/%s-%d.log", get_macaron_proxy_name(), record_idx++);
                FILE *ofp = fopen(ofname, "w");
                char header[] = "timestamp(ms),op,key,size,imc(us),oscm(us),osc(us),dl(us),end(us),getsrc\n";
                fwrite(header, strlen(header), 1, ofp);
                for (int i = *(ctx->wbeg); i != *(ctx->wend); i = (i + 1) % record_buffer_size) {
                    struct request_record *line = &(*ctx->buffer)[i];
                    snprintf(linestr, 300, "%lu,%" PRIu8 ",%s,%" PRIu32 ",%" PRIu32 ",%" PRIu32 ",%" PRIu32 ",%" PRIu32 ",%" PRIu32 ",%" PRIu8 "\n",
                             line->ts, line->op, line->key, line->size, line->imc, line->oscm, line->osc, line->dl, line->end, line->get_src);
                    fwrite(linestr, strlen(linestr), 1, ofp);
                }
                fclose(ofp);

                char cmd[200];
                snprintf(cmd, 200, "mv %s /tmp/mac_records/mid", ofname);
                system(cmd);
            }
            *(ctx->wbeg) = *(ctx->wend);

            // update the new record time
            record_time.tv_sec = curr_time.tv_sec;
            record_time.tv_usec = curr_time.tv_usec;
        }
        sleep(1);
    }
    nc_free(ofname);
    nc_free(linestr);
    pthread_exit(NULL);
}

void add_record(uint8_t op, char *key, size_t keylen, size_t vallen, uint32_t imc_latency, uint32_t oscm_latency,
                uint32_t osc_latency, uint32_t dl_latency, uint32_t end_latency, uint8_t get_src) {
    int next_record_buffer_curr = (record_buffer_curr + 1) % record_buffer_size;
    ASSERT(next_record_buffer_curr != record_buffer_wbeg);
    struct timeval curr_time;
    gettimeofday(&curr_time, NULL);
    record_buffer[record_buffer_curr].ts =
        (unsigned long)curr_time.tv_sec * (unsigned long)1000 + (unsigned long)(curr_time.tv_usec / 1000);  // ms
    record_buffer[record_buffer_curr].op = (uint8_t)op;                                                     // PUT: 0, GET: 1, DELETE: 2
    strncpy(record_buffer[record_buffer_curr].key, key, keylen);
    record_buffer[record_buffer_curr].key[keylen] = '\0';
    record_buffer[record_buffer_curr].size = vallen;
    record_buffer[record_buffer_curr].imc = imc_latency;
    record_buffer[record_buffer_curr].oscm = oscm_latency;
    record_buffer[record_buffer_curr].osc = osc_latency;
    record_buffer[record_buffer_curr].dl = dl_latency;
    record_buffer[record_buffer_curr].end = end_latency;
    record_buffer[record_buffer_curr].get_src = get_src;

    record_buffer_curr = next_record_buffer_curr;
}

/* End of record related variables and functions*/

#define DEFINE_ACTION(_name) string(#_name),
static const struct string msg_type_strings[] = {
    MSG_TYPE_CODEC(DEFINE_ACTION)
        null_string};
#undef DEFINE_ACTION

static struct msg *
msg_from_rbe(struct rbnode *node) {
    struct msg *msg;
    int offset;

    offset = offsetof(struct msg, tmo_rbe);
    msg = (struct msg *)((char *)node - offset);

    return msg;
}

struct msg *
msg_tmo_min(void) {
    struct rbnode *node;

    node = rbtree_min(&tmo_rbt);
    if (node == NULL) {
        return NULL;
    }

    return msg_from_rbe(node);
}

void msg_tmo_insert(struct msg *msg, struct conn *conn) {
    struct rbnode *node;
    int timeout;

    ASSERT(msg->request);
    ASSERT(!msg->quit && !msg->noreply);

    timeout = server_timeout(conn);
    if (timeout <= 0) {
        return;
    }

    node = &msg->tmo_rbe;
    node->key = nc_msec_now() + timeout;
    node->data = conn;

    rbtree_insert(&tmo_rbt, node);

    log_debug(LOG_VERB, "insert msg %" PRIu64
                        " into tmo rbt with expiry of "
                        "%d msec",
              msg->id, timeout);
}

void msg_tmo_delete(struct msg *msg) {
    struct rbnode *node;

    node = &msg->tmo_rbe;

    /* already deleted */

    if (node->data == NULL) {
        return;
    }

    rbtree_delete(&tmo_rbt, node);

    log_debug(LOG_VERB, "delete msg %" PRIu64 " from tmo rbt", msg->id);
}

static struct msg *
_msg_get(void) {
    struct msg *msg;

    if (!TAILQ_EMPTY(&free_msgq)) {
        ASSERT(nfree_msgq > 0);

        msg = TAILQ_FIRST(&free_msgq);
        nfree_msgq--;
        TAILQ_REMOVE(&free_msgq, msg, m_tqe);
        goto done;
    }

    msg = nc_alloc(sizeof(*msg));
    if (msg == NULL) {
        return NULL;
    }

done:
    /* c_tqe, s_tqe, and m_tqe are left uninitialized */
    msg->id = ++msg_id;
    msg->peer = NULL;
    msg->owner = NULL;

    rbtree_node_init(&msg->tmo_rbe);

    STAILQ_INIT(&msg->mhdr);
    msg->mlen = 0;
    msg->start_ts = 0;

    msg->state = 0;
    msg->pos = NULL;
    msg->token = NULL;

    msg->parser = NULL;
    msg->add_auth = NULL;
    msg->result = MSG_PARSE_OK;

    msg->fragment = NULL;
    msg->reply = NULL;
    msg->pre_coalesce = NULL;
    msg->post_coalesce = NULL;

    msg->type = MSG_UNKNOWN;

    msg->keys = array_create(1, sizeof(struct keypos));
    if (msg->keys == NULL) {
        nc_free(msg);
        return NULL;
    }

    msg->vlen = 0;
    msg->end = NULL;

    msg->frag_owner = NULL;
    msg->frag_seq = NULL;
    msg->nfrag = 0;
    msg->nfrag_done = 0;
    msg->frag_id = 0;

    msg->narg_start = NULL;
    msg->narg_end = NULL;
    msg->narg = 0;
    msg->rnarg = 0;
    msg->rlen = 0;
    /*
     * This is used for both parsing redis responses
     * and as a counter for coalescing responses such as DEL
     */
    msg->integer = 0;

    msg->err = 0;
    msg->error = 0;
    msg->ferror = 0;
    msg->request = 0;
    msg->quit = 0;
    msg->noreply = 0;
    msg->noforward = 0;
    msg->done = 0;
    msg->fdone = 0;
    msg->swallow = 0;
    msg->redis = 0;

    return msg;
}

struct msg *
msg_get(struct conn *conn, bool request, bool redis) {
    struct msg *msg;

    msg = _msg_get();
    if (msg == NULL) {
        return NULL;
    }

    msg->owner = conn;
    msg->request = request ? 1 : 0;
    msg->redis = redis ? 1 : 0;

    if (redis) {
        if (request) {
            msg->parser = redis_parse_req;
        } else {
            msg->parser = redis_parse_rsp;
        }
        msg->add_auth = redis_add_auth;
        msg->fragment = redis_fragment;
        msg->reply = redis_reply;
        msg->failure = redis_failure;
        msg->pre_coalesce = redis_pre_coalesce;
        msg->post_coalesce = redis_post_coalesce;
    } else {
        if (request) {
            msg->parser = memcache_parse_req;
        } else {
            msg->parser = memcache_parse_rsp;
        }
        msg->add_auth = memcache_add_auth;
        msg->fragment = memcache_fragment;
        msg->failure = memcache_failure;
        msg->pre_coalesce = memcache_pre_coalesce;
        msg->post_coalesce = memcache_post_coalesce;
    }

    if (log_loggable(LOG_NOTICE) != 0) {
        msg->start_ts = nc_usec_now();
    }

    log_debug(LOG_VVERB, "get msg %p id %" PRIu64 " request %d owner sd %d",
              msg, msg->id, msg->request, conn->sd);

    return msg;
}

struct msg *
msg_get_error(bool redis, err_t err) {
    struct msg *msg;
    struct mbuf *mbuf;
    int n;
    const char *errstr = err ? strerror(err) : "unknown";
    const char *protstr = redis ? "-ERR" : "SERVER_ERROR";

    msg = _msg_get();
    if (msg == NULL) {
        return NULL;
    }

    msg->state = 0;
    msg->type = MSG_RSP_MC_SERVER_ERROR;

    mbuf = mbuf_get();
    if (mbuf == NULL) {
        msg_put(msg);
        return NULL;
    }
    mbuf_insert(&msg->mhdr, mbuf);

    n = nc_scnprintf(mbuf->last, mbuf_size(mbuf), "%s %s" CRLF, protstr, errstr);
    mbuf->last += n;
    msg->mlen = (uint32_t)n;

    log_debug(LOG_VVERB, "get msg %p id %" PRIu64 " len %" PRIu32 " error '%s'",
              msg, msg->id, msg->mlen, errstr);

    return msg;
}

static void
msg_free(struct msg *msg) {
    ASSERT(STAILQ_EMPTY(&msg->mhdr));

    log_debug(LOG_VVERB, "free msg %p id %" PRIu64 "", msg, msg->id);
    nc_free(msg);
}

void msg_put(struct msg *msg) {
    log_debug(LOG_VVERB, "put msg %p id %" PRIu64 "", msg, msg->id);

    while (!STAILQ_EMPTY(&msg->mhdr)) {
        struct mbuf *mbuf = STAILQ_FIRST(&msg->mhdr);
        mbuf_remove(&msg->mhdr, mbuf);
        mbuf_put(mbuf);
    }

    if (msg->frag_seq) {
        nc_free(msg->frag_seq);
        msg->frag_seq = NULL;
    }

    if (msg->keys) {
        msg->keys->nelem = 0; /* a hack here */
        array_destroy(msg->keys);
        msg->keys = NULL;
    }

    nfree_msgq++;
    TAILQ_INSERT_HEAD(&free_msgq, msg, m_tqe);
}

void msg_dump(const struct msg *msg, int level) {
    const struct mbuf *mbuf;

    if (log_loggable(level) == 0) {
        return;
    }

    loga("msg dump id %" PRIu64 " request %d len %" PRIu32
         " type %d done %d "
         "error %d (err %d)",
         msg->id, msg->request, msg->mlen, msg->type,
         msg->done, msg->error, msg->err);

    STAILQ_FOREACH(mbuf, &msg->mhdr, next) {
        uint8_t *p, *q;
        long int len;

        p = mbuf->start;
        q = mbuf->last;
        len = q - p;

        loga_hexdump(p, len, "mbuf [%p] with %ld bytes of data", p, len);
    }
}

void msg_init(void) {
    log_debug(LOG_DEBUG, "msg size %d", (int)sizeof(struct msg));
    msg_id = 0;
    frag_id = 0;
    nfree_msgq = 0;
    TAILQ_INIT(&free_msgq);
    rbtree_init(&tmo_rbt, &tmo_rbs);
}

void msg_deinit(void) {
    struct msg *msg, *nmsg;

    for (msg = TAILQ_FIRST(&free_msgq); msg != NULL;
         msg = nmsg, nfree_msgq--) {
        ASSERT(nfree_msgq > 0);
        nmsg = TAILQ_NEXT(msg, m_tqe);
        msg_free(msg);
    }
    ASSERT(nfree_msgq == 0);
}

const struct string *
msg_type_string(msg_type_t type) {
    return &msg_type_strings[type];
}

bool msg_empty(const struct msg *msg) {
    return msg->mlen == 0;
}

uint32_t
msg_backend_idx(const struct msg *msg, const uint8_t *key, uint32_t keylen) {
    struct conn *conn = msg->owner;
    struct server_pool *pool = conn->owner;

    return server_pool_idx(pool, key, keylen);
}

struct mbuf *
msg_ensure_mbuf(struct msg *msg, size_t len) {
    struct mbuf *mbuf;

    if (STAILQ_EMPTY(&msg->mhdr) ||
        mbuf_size(STAILQ_LAST(&msg->mhdr, mbuf, next)) < len) {
        mbuf = mbuf_get();
        if (mbuf == NULL) {
            return NULL;
        }
        mbuf_insert(&msg->mhdr, mbuf);
    } else {
        mbuf = STAILQ_LAST(&msg->mhdr, mbuf, next);
    }

    return mbuf;
}

/*
 * Append n bytes of data, with n <= mbuf_size(mbuf)
 * into mbuf
 */
rstatus_t
msg_append(struct msg *msg, const uint8_t *pos, size_t n) {
    struct mbuf *mbuf;

    ASSERT(n <= mbuf_data_size());

    mbuf = msg_ensure_mbuf(msg, n);
    if (mbuf == NULL) {
        return NC_ENOMEM;
    }

    ASSERT(n <= mbuf_size(mbuf));

    mbuf_copy(mbuf, pos, n);
    msg->mlen += (uint32_t)n;

    return NC_OK;
}

/*
 * Prepend n bytes of data, with n <= mbuf_size(mbuf)
 * into mbuf
 */
rstatus_t
msg_prepend(struct msg *msg, const uint8_t *pos, size_t n) {
    struct mbuf *mbuf;

    mbuf = mbuf_get();
    if (mbuf == NULL) {
        return NC_ENOMEM;
    }

    ASSERT(n <= mbuf_size(mbuf));

    mbuf_copy(mbuf, pos, n);
    msg->mlen += (uint32_t)n;

    STAILQ_INSERT_HEAD(&msg->mhdr, mbuf, next);

    return NC_OK;
}

/*
 * Prepend a formatted string into msg. Returns an error if the formatted
 * string does not fit in a single mbuf.
 */
rstatus_t
msg_prepend_format(struct msg *msg, const char *fmt, ...) {
    struct mbuf *mbuf;
    int n;
    uint32_t size;
    va_list args;

    mbuf = mbuf_get();
    if (mbuf == NULL) {
        return NC_ENOMEM;
    }

    size = mbuf_size(mbuf);

    va_start(args, fmt);
    n = nc_vsnprintf(mbuf->last, size, fmt, args);
    va_end(args);
    if (n <= 0 || n >= (int)size) {
        return NC_ERROR;
    }

    mbuf->last += n;
    msg->mlen += (uint32_t)n;
    STAILQ_INSERT_HEAD(&msg->mhdr, mbuf, next);

    return NC_OK;
}

inline uint64_t
msg_gen_frag_id(void) {
    return ++frag_id;
}

static rstatus_t
msg_parsed(struct context *ctx, struct conn *conn, struct msg *msg) {
    struct msg *nmsg;
    struct mbuf *mbuf, *nbuf;

    mbuf = STAILQ_LAST(&msg->mhdr, mbuf, next);
    if (msg->pos == mbuf->last) {
        /* no more data to parse */
        if (!conn->client) {
            loga("[nc_message.msg_parsed] No more data to parse");
        }
        conn->recv_done(ctx, conn, msg, NULL);
        return NC_OK;
    }

    /*
     * Input mbuf has un-parsed data. Split mbuf of the current message msg
     * into (mbuf, nbuf), where mbuf is the portion of the message that has
     * been parsed and nbuf is the portion of the message that is un-parsed.
     * Parse nbuf as a new message nmsg in the next iteration.
     */
    nbuf = mbuf_split(&msg->mhdr, msg->pos, NULL, NULL);
    if (nbuf == NULL) {
        return NC_ENOMEM;
    }

    nmsg = msg_get(msg->owner, msg->request, conn->redis);
    if (nmsg == NULL) {
        mbuf_put(nbuf);
        return NC_ENOMEM;
    }
    mbuf_insert(&nmsg->mhdr, nbuf);
    nmsg->pos = nbuf->pos;

    /* update length of current (msg) and new message (nmsg) */
    nmsg->mlen = mbuf_length(nbuf);
    msg->mlen -= nmsg->mlen;

    conn->recv_done(ctx, conn, msg, nmsg);

    return NC_OK;
}

static rstatus_t
msg_repair(struct context *ctx, struct conn *conn, struct msg *msg) {
    struct mbuf *nbuf;

    nbuf = mbuf_split(&msg->mhdr, msg->pos, NULL, NULL);
    if (nbuf == NULL) {
        return NC_ENOMEM;
    }
    mbuf_insert(&msg->mhdr, nbuf);
    msg->pos = nbuf->pos;

    return NC_OK;
}

static rstatus_t
msg_parse(struct context *ctx, struct conn *conn, struct msg *msg) {
    rstatus_t status;

    if (msg_empty(msg)) {
        /* no data to parse */
        conn->recv_done(ctx, conn, msg, NULL);
        return NC_OK;
    }

    msg->parser(msg);

    switch (msg->result) {
        case MSG_PARSE_OK:
            status = msg_parsed(ctx, conn, msg);
            break;

        case MSG_PARSE_REPAIR:
            status = msg_repair(ctx, conn, msg);
            break;

        case MSG_PARSE_AGAIN:
            status = NC_OK;
            break;

        default:
            status = NC_ERROR;
            conn->err = errno;
            break;
    }

    return conn->err != 0 ? NC_ERROR : status;
}

static rstatus_t
msg_recv_chain(struct context *ctx, struct conn *conn, struct msg *msg) {
    rstatus_t status;
    struct msg *nmsg;
    struct mbuf *mbuf;
    size_t msize;
    ssize_t n;

    mbuf = STAILQ_LAST(&msg->mhdr, mbuf, next);
    if (mbuf == NULL || mbuf_full(mbuf)) {
        mbuf = mbuf_get();
        if (mbuf == NULL) {
            return NC_ENOMEM;
        }
        mbuf_insert(&msg->mhdr, mbuf);
        msg->pos = mbuf->pos;
    }
    ASSERT(mbuf->end - mbuf->last > 0);

    msize = mbuf_size(mbuf);

    n = conn_recv(conn, mbuf->last, msize);
    if (n < 0) {
        if (n == NC_EAGAIN) {
            return NC_OK;
        }
        return NC_ERROR;
    }

    ASSERT((mbuf->last + n) <= mbuf->end);
    mbuf->last += n;
    msg->mlen += (uint32_t)n;

    for (;;) {
        if (conn->proxy) {
            // Do nothing
        }
        if (!conn->client) {
            loga("[nc_message.msg_rcv_chain] Server: For loop");
        } else {
            loga("[nc_message.msg_rcv_chain] Client: For loop");
        }

        status = msg_parse(ctx, conn, msg);
        if (status != NC_OK) {
            return status;
        }

        /* get next message to parse */
        nmsg = conn->recv_next(ctx, conn, false);
        if (nmsg == NULL || nmsg == msg) {
            /* no more data to parse */
            break;
        }

        msg = nmsg;
    }

    return NC_OK;
}

rstatus_t
msg_recv(struct context *ctx, struct conn *conn) {
    loga("[nc_message.msg_recv] received message");
    rstatus_t status;
    struct msg *msg;

    ASSERT(conn->recv_active);

    conn->recv_ready = 1;
    do {
        msg = conn->recv_next(ctx, conn, true);
        if (msg == NULL) {
            return NC_OK;
        }

        status = msg_recv_chain(ctx, conn, msg);
        if (status != NC_OK) {
            return status;
        }
    } while (conn->recv_ready);

    return NC_OK;
}

static int
try_get_data_from_datalake(char *key, char **new_msg, int *new_msg_len, int *data_offset, int *data_size) {
    loga("[try_get_data_from_datalake] Start");
    int ret = aws_get_data_from_datalake(key, new_msg, new_msg_len, data_offset, data_size);
    return ret;
}

static void
try_get_data_from_osc(struct context *ctx, struct conn *conn, struct msg *msg) {
    loga("[try_get_data_from_osc] Start");

    struct mbuf *mbuf, *nbuf;            /* current and next mbuf */
    size_t mlen;                         /* current mbuf data length */
    struct iovec *ciov, iov[NC_IOV_MAX]; /* current iovec */
    struct array sendv;                  /* send iovec */
    size_t nsend;                        /* bytes to send; bytes sent */
    size_t limit;                        /* bytes to send limit */

    // Data to be recorded
    uint32_t imc_latency = get_elapsed_time(msg), oscm_latency = 0, osc_latency = 0,
             dl_latency = 0, end_latency = 0;
    size_t record_data_size;
    uint32_t curr_elapsed_time = imc_latency, new_elapsed_time;
    uint8_t get_src;

    char *key = redis_parse_peer_msg_get_key(msg);
    ASSERT(key != NULL);
    struct oscm_result *oscm_md = oscm_get_metadata(key);
    new_elapsed_time = get_elapsed_time(msg);
    oscm_latency = new_elapsed_time - curr_elapsed_time;
    curr_elapsed_time = new_elapsed_time;

    ASSERT(oscm_md != NULL);
    if (oscm_md->exist) {
        // If data is in the Object Storage Cache, get data from OSC
        // "$" "[length]" "CRLF" "[data]" "CRLF"
        int value_str_len = (int)log10(abs(oscm_md->size)) + 1;
        int new_msg_len = 5 + oscm_md->size + value_str_len;

        int offset = 0;
        char *new_msg = (char *)nc_alloc(new_msg_len + 1);
        new_msg[0] = '$';
        new_msg[new_msg_len] = '\0';
        offset += 1;
        snprintf(new_msg + offset, (size_t)new_msg_len, "%d", oscm_md->size);
        offset += value_str_len;
        new_msg[offset] = '\r';
        new_msg[offset + 1] = '\n';
        offset += 2;
        int data_offset = offset;
        aws_get_data_from_osc(oscm_md->block_id, oscm_md->offset, oscm_md->size, new_msg + offset, new_msg_len);
        offset += oscm_md->size;
        new_msg[offset] = '\r';
        new_msg[offset + 1] = '\n';
        offset += 2;
        ASSERT(offset == new_msg_len);
        new_elapsed_time = get_elapsed_time(msg);
        osc_latency = new_elapsed_time - curr_elapsed_time;
        curr_elapsed_time = new_elapsed_time;

        // Send new_msg to client
        array_set(&sendv, iov, sizeof(iov[0]), NC_IOV_MAX);

        ciov = array_push(&sendv);
        ciov->iov_base = new_msg;
        ciov->iov_len = (size_t)new_msg_len;
        nsend = (size_t)new_msg_len;

        conn->smsg = NULL;
        conn_sendv(conn, &sendv, nsend);
        end_latency = get_elapsed_time(msg);
        record_data_size = oscm_md->size;
        get_src = (uint8_t)1;

        // Promote data to Redis server
        redis_put_data(key, new_msg + data_offset, oscm_md->size);

        nc_free(new_msg);
    } else {
        // If data is not in OSC, try getting data from Datalake.
        loga("[try_get_data_from_osc] osc_md does not exist for %s", key);
        char *new_msg = NULL;
        int new_msg_len = 0;
        int data_offset, data_size;
        int exists = try_get_data_from_datalake(key, &new_msg, &new_msg_len, &data_offset, &data_size);
        new_elapsed_time = get_elapsed_time(msg);
        dl_latency = new_elapsed_time - curr_elapsed_time;
        curr_elapsed_time = new_elapsed_time;
        get_src = (uint8_t)2;
        if (exists) {
            // data is in datalake.
            loga("[try_get_data_from_datalake] Data is in the datalake.");
            ASSERT(new_msg != NULL);

            // Send new_msg to client
            array_set(&sendv, iov, sizeof(iov[0]), NC_IOV_MAX);

            ciov = array_push(&sendv);
            ciov->iov_base = new_msg;
            ciov->iov_len = (size_t)new_msg_len;
            nsend = (size_t)new_msg_len;

            conn->smsg = NULL;
            conn_sendv(conn, &sendv, nsend);
            end_latency = get_elapsed_time(msg);
            record_data_size = data_size;

            // Promote data to OSC and Redis server
            aws_put_data_to_osc_packing(key, new_msg + data_offset, data_size);
            redis_put_data(key, new_msg + data_offset, data_size);

            nc_free(new_msg);
        } else {
            // If data is not is Datalake as well, send the "$-1" message (original message)
            loga("Data is not in the datalake, too. Respond with $-1 message");
            array_set(&sendv, iov, sizeof(iov[0]), NC_IOV_MAX);

            nsend = 0;
            limit = SSIZE_MAX;
            // ASSERT(conn->smsg == msg);
            for (mbuf = STAILQ_FIRST(&msg->mhdr);
                 mbuf != NULL && array_n(&sendv) < NC_IOV_MAX && nsend < limit;
                 mbuf = nbuf) {
                nbuf = STAILQ_NEXT(mbuf, next);
                if (mbuf_empty(mbuf)) {
                    continue;
                }
                mlen = mbuf_length(mbuf);
                if ((nsend + mlen) > limit) {
                    mlen = limit - nsend;
                }
                ciov = array_push(&sendv);
                ciov->iov_base = mbuf->pos;
                ciov->iov_len = mlen;
                nsend += mlen;
            }
            loga("[try_get_data_from_datalake] nsend: %zu", nsend);
            conn->smsg = NULL;
            conn_sendv(conn, &sendv, nsend);
            end_latency = get_elapsed_time(msg);
            record_data_size = 0;
        }
    }
    add_record((uint8_t)1, key, strlen(key), record_data_size, imc_latency, oscm_latency, osc_latency, dl_latency,
               end_latency, get_src);
    nc_free(key);
    nc_free(oscm_md);

    /* adjust mbufs of the sent message */
    for (mbuf = STAILQ_FIRST(&msg->mhdr); mbuf != NULL; mbuf = nbuf) {
        nbuf = STAILQ_NEXT(mbuf, next);
        if (mbuf_empty(mbuf)) {
            continue;
        }
        mbuf->pos = mbuf->last;
    }

    ASSERT(mbuf == NULL);
    conn->send_done(ctx, conn, msg);
}

static rstatus_t
msg_send_chain(struct context *ctx, struct conn *conn, struct msg *msg) {
    struct msg_tqh send_msgq;            /* send msg q */
    struct msg *nmsg;                    /* next msg */
    struct mbuf *mbuf, *nbuf;            /* current and next mbuf */
    size_t mlen;                         /* current mbuf data length */
    struct iovec *ciov, iov[NC_IOV_MAX]; /* current iovec */
    struct array sendv;                  /* send iovec */
    size_t nsend, nsent;                 /* bytes to send; bytes sent */
    size_t limit;                        /* bytes to send limit */
    ssize_t n;                           /* bytes sent by sendv */

    TAILQ_INIT(&send_msgq);

    array_set(&sendv, iov, sizeof(iov[0]), NC_IOV_MAX);

    /* preprocess - build iovec */

    nsend = 0;
    /*
     * readv() and writev() returns EINVAL if the sum of the iov_len values
     * overflows an ssize_t value Or, the vector count iovcnt is less than
     * zero or greater than the permitted maximum.
     */
    limit = SSIZE_MAX;

    for (;;) {
        ASSERT(conn->smsg == msg);

        /* Macaron: if there is no data in the Redis server, reach out to the local/remote object storage */
        bool insert_flag = true;
        struct msg *pmsg = msg->peer;
        if (msg != NULL && pmsg != NULL && pmsg->owner != NULL) {
            ASSERT(msg->redis);       /* We implemented Macaron for only Redis */
            struct mbuf *xbuf = NULL; /* mbuf to be used for "$-1" checker (Macaron) */
            if (pmsg->type == 50) {
                xbuf = STAILQ_FIRST(&msg->mhdr);
                if (str3icmp(xbuf->start, '$', '-', '1')) {
                    loga("[msg_send_chain] Failed to retrieve data: no such data is in the Redis server.");
                    try_get_data_from_osc(ctx, conn, msg);
                    insert_flag = false;
                }
            }

            if (insert_flag && (pmsg->type == 29 || pmsg->type == 50 || pmsg->type == 63)) {  // Get: 50, Del: 29, Set: 63
                ASSERT(array_n(pmsg->keys) > 0);
                struct keypos *kpos = array_get(pmsg->keys, 0);
                uint8_t *key = kpos->start;
                uint32_t keylen = (uint32_t)(kpos->end - kpos->start);
                loga("[msg_send_chain] MsgTypeEnum: %d, Keylen: %d, key: %.*s", pmsg->type, keylen, keylen, key);
                if (pmsg->type == 63) {  // PUT
                    uint8_t *last_pos = kpos->end + 2;
                    while ((uint8_t)(*last_pos) != 10)
                        last_pos++;
                    uint32_t data_size = pmsg->mlen - 19 - keylen - log10(keylen) - (last_pos - kpos->end);
                    loga("[msg_send_chain] Put mlen: %d, data size: %zu", pmsg->mlen, data_size);
                    add_record((uint8_t)0, key, keylen, data_size, 0, 0, 0, 0, 0, (uint8_t)0);
                } else if (pmsg->type == 50) {  // GET
                    // loga("[msg_send_chain] Get mlen: %d, data size: %zu", pmsg->mlen, data_size);
                    ASSERT(xbuf != NULL);
                    uint8_t *last_pos = xbuf->start;
                    while ((uint8_t)(*last_pos) != 10)
                        last_pos++;
                    size_t data_size = msg->mlen - (last_pos - xbuf->start + 3);
                    loga("[msg_send_chain] Get mlen: %d, data size: %zu", msg->mlen, data_size);
                    uint32_t elapsed_time = get_elapsed_time(msg);
                    add_record((uint8_t)1, key, keylen, data_size, elapsed_time, 0, 0, 0, elapsed_time, 0);
                } else if (pmsg->type == 29) {
                    loga("[msg_send_chain] Delete for key: %.*s", keylen, key);
                    add_record((uint8_t)2, key, keylen, 0, 0, 0, 0, 0, 0, 0);
                }
            }
        }

        if (insert_flag) {
            TAILQ_INSERT_TAIL(&send_msgq, msg, m_tqe);

            for (mbuf = STAILQ_FIRST(&msg->mhdr);
                 mbuf != NULL && array_n(&sendv) < NC_IOV_MAX && nsend < limit;
                 mbuf = nbuf) {
                nbuf = STAILQ_NEXT(mbuf, next);

                if (mbuf_empty(mbuf)) {
                    continue;
                }

                mlen = mbuf_length(mbuf);
                if ((nsend + mlen) > limit) {
                    mlen = limit - nsend;
                }

                ciov = array_push(&sendv);
                ciov->iov_base = mbuf->pos;
                ciov->iov_len = mlen;

                nsend += mlen;
            }

            if (array_n(&sendv) >= NC_IOV_MAX || nsend >= limit) {
                break;
            }
        }

        msg = conn->send_next(ctx, conn);
        if (msg == NULL) {
            break;
        }
    }

    /*
     * (nsend == 0) is possible in redis multi-del
     * see PR: https://github.com/twitter/twemproxy/pull/225
     */
    conn->smsg = NULL;
    if (!TAILQ_EMPTY(&send_msgq) && nsend != 0) {
        n = conn_sendv(conn, &sendv, nsend);
    } else {
        n = 0;
    }

    nsent = n > 0 ? (size_t)n : 0;

    /* postprocess - process sent messages in send_msgq */

    for (msg = TAILQ_FIRST(&send_msgq); msg != NULL; msg = nmsg) {
        nmsg = TAILQ_NEXT(msg, m_tqe);

        TAILQ_REMOVE(&send_msgq, msg, m_tqe);

        if (nsent == 0) {
            if (msg->mlen == 0) {
                conn->send_done(ctx, conn, msg);
            }
            continue;
        }

        /* adjust mbufs of the sent message */
        for (mbuf = STAILQ_FIRST(&msg->mhdr); mbuf != NULL; mbuf = nbuf) {
            nbuf = STAILQ_NEXT(mbuf, next);

            if (mbuf_empty(mbuf)) {
                continue;
            }

            mlen = mbuf_length(mbuf);
            if (nsent < mlen) {
                /* mbuf was sent partially; process remaining bytes later */
                mbuf->pos += nsent;
                ASSERT(mbuf->pos < mbuf->last);
                nsent = 0;
                break;
            }

            /* mbuf was sent completely; mark it empty */
            mbuf->pos = mbuf->last;
            nsent -= mlen;
        }

        /* message has been sent completely, finalize it */
        if (mbuf == NULL) {
            conn->send_done(ctx, conn, msg);
        }
    }

    ASSERT(TAILQ_EMPTY(&send_msgq));

    if (n >= 0) {
        return NC_OK;
    }

    return (n == NC_EAGAIN) ? NC_OK : NC_ERROR;
}

rstatus_t
msg_send(struct context *ctx, struct conn *conn) {
    rstatus_t status;
    struct msg *msg;

    ASSERT(conn->send_active);

    conn->send_ready = 1;
    do {
        msg = conn->send_next(ctx, conn);
        if (msg == NULL) {
            /* nothing to send */
            return NC_OK;
        }

        status = msg_send_chain(ctx, conn, msg);
        if (status != NC_OK) {
            return status;
        }

    } while (conn->send_ready);

    return NC_OK;
}

/*
 * Set a placeholder key for a command with no key that is forwarded to an
 * arbitrary backend.
 */
bool msg_set_placeholder_key(struct msg *r) {
    struct keypos *kpos;
    ASSERT(array_n(r->keys) == 0);
    kpos = array_push(r->keys);
    if (kpos == NULL) {
        return false;
    }
    kpos->start = (uint8_t *)"placeholder";
    kpos->end = kpos->start + sizeof("placeholder") - 1;
    return true;
}
