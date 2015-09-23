#ifndef QTELNET_H
#define QTELNET_H

#include <string>
#include <termios.h>
#include <libtelnet.h>


using namespace std;

class qtelnet
{
public:
    /**
     * @brief telnet_connect Open socket and onnect to telnet server
     * @param tracker
     * @param host
     * @param port Default 23
     * @return
     */
    static int telnet_connect(qtelnet &tracker,
                              const char *host,
                              const char *port = string("23").c_str());
    static void telnet_disconnect(qtelnet &tracker);

    qtelnet();
    ~qtelnet();

private:
    int sockfd;
    termios *orig_tios; // struct termios
    telnet_t *telnet; // Telnet state box
    int do_echo;

    /**
     * @brief telnet_event_handler
     * @param telnet The telnet object that generated the event
     * @param event Event structure with details about the event
     * @param user_data User-supplied pointer
     */
    static void telnet_event_handler(telnet_t *telnet,
                                     telnet_event_t *event,
                                     void *user_data);
    /**
     * @brief send Send data over socket
     * @param tracker
     * @param buffer
     * @param size
     * @return Number of sent byte; -1 on error
     */
    static int send_data(qtelnet *tracker, const char *buffer, size_t size);
};

#endif // QTELNET_H
