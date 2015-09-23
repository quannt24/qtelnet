#include "qtelnet.h"

#include <iostream>
#include <iomanip>
#include <netdb.h>
#include <cstring>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <cstdlib>


using namespace std;

int qtelnet::telnet_connect(qtelnet &tracker,
                            const char *host,
                            const char *port)
{
    struct addrinfo hints;
    struct addrinfo *ai;
    int sock;
    int rs;
    struct termios *orig_tios = new termios;
    telnet_t *telnet;

    /* look up server host */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if ((rs = getaddrinfo(host, port, &hints, &ai)) != 0) {
        cerr << "getaddrinfo() failed for " << host << ": " << gai_strerror(rs);
        return 1;
    }

    /* create socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        cerr << "socket() failed: " << strerror(errno) << "\n";
        return 2;
    }

    /* connect */
    if (connect(sock, ai->ai_addr, ai->ai_addrlen) == -1) {
        cerr << "connect() failed: " << strerror(errno) << "\n";
        return 3;
    }

    /* free address lookup info */
    freeaddrinfo(ai);

    /* initialize telnet box */
    telnet_telopt_t telopts[] = {
        { TELNET_TELOPT_ECHO,		TELNET_WONT, TELNET_DO   },
        { TELNET_TELOPT_TTYPE,		TELNET_WILL, TELNET_DONT },
        { TELNET_TELOPT_COMPRESS2,	TELNET_WONT, TELNET_DO   },
        { TELNET_TELOPT_MSSP,		TELNET_WONT, TELNET_DO   },
        { -1, 0, 0 }
    };
    telnet = telnet_init(telopts, telnet_event_handler, 0, &tracker);

    tracker.sockfd = sock;
    tracker.orig_tios = orig_tios;
    tracker.telnet = telnet;

    return 0;
}

void qtelnet::telnet_disconnect(qtelnet &tracker)
{
    if (tracker.sockfd != -1) {
        close(tracker.sockfd);
        tracker.sockfd = -1;
    }
    if (tracker.telnet != NULL) {
        telnet_free(tracker.telnet);
        tracker.telnet = NULL;
    }
}

qtelnet::qtelnet()
{
    struct termios tios;

    sockfd = -1;
    orig_tios = new termios;
    do_echo = 1;

    /* get current terminal settings, set raw mode, make sure
     * to restore terminal settings */
    tcgetattr(STDOUT_FILENO, orig_tios);
    tios = *orig_tios;
    cfmakeraw(&tios);
    tcsetattr(STDOUT_FILENO, TCSADRAIN, &tios);
}

qtelnet::~qtelnet()
{
    if (orig_tios != NULL) {
        tcsetattr(STDOUT_FILENO, TCSADRAIN, orig_tios);
    }
}

void qtelnet::telnet_event_handler(telnet_t *telnet,
                                   telnet_event_t *event,
                                   void *user_data)
{
    qtelnet *tracker = (qtelnet*) user_data;

    switch (event->type) {
    case TELNET_EV_DATA:
        /* data received */
        cout << setw(event->data.size) << event->data.buffer << "\n";
        break;

    case TELNET_EV_SEND:
        /* data must be sent */
        send_data(tracker, event->data.buffer, event->data.size);
        break;

    case TELNET_EV_WILL:
        /* request to enable remote feature (or receipt) */
        /* we'll agree to turn off our echo if server wants us to stop */
        if (event->neg.telopt == TELNET_TELOPT_ECHO)
            tracker->do_echo = 0;
        break;

    case TELNET_EV_WONT:
        /* notification of disabling remote feature (or receipt) */
        if (event->neg.telopt == TELNET_TELOPT_ECHO)
            tracker->do_echo = 1;
        break;

    case TELNET_EV_DO:
        /* request to enable local feature (or receipt) */
        break;

    case TELNET_EV_DONT:
        /* demand to disable local feature (or receipt) */
        break;

    case TELNET_EV_TTYPE:
        /* respond to TTYPE commands */
        /* respond with our terminal type, if requested */
        if (event->ttype.cmd == TELNET_TTYPE_SEND) {
            telnet_ttype_is(telnet, getenv("TERM"));
        }
        break;

    case TELNET_EV_SUBNEGOTIATION:
        /* respond to particular subnegotiations */
        break;

    case TELNET_EV_ERROR:
        /* error */
        cerr << "ERROR: " << event->error.msg << "\n";
        break;

    default:
        /* ignore */
        break;
    }
}

int qtelnet::send_data(qtelnet *tracker, const char *buffer, size_t size)
{
    int rs;
    size_t len = size;

    /* send data */
    while (len > 0) {
        if ((rs = send(tracker->sockfd, buffer, len, 0)) == -1) {
            cerr << "send() failed: " << strerror(errno) << "\n";
            return -1;
        } else if (rs == 0) {
            cerr << "send() unexpectedly returned 0\n";
            return -1;
        }

        /* update pointer and size to see if we've got more to send */
        buffer += rs;
        len -= rs;
    }

    return (size - len);
}
