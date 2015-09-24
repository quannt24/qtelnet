﻿#ifndef QTELNET_H
#define QTELNET_H

#include <string>
#include <termios.h>
#include <libtelnet.h>
#include <pthread.h>


using namespace std;

class qtelnet
{
public:
    typedef void (*data_recv_callback_t)(const char *data, int size, void *bundle);

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

    void set_data_recv_callback(const data_recv_callback_t callback);
    void set_data_recv_bundle(void *bundle);

private:
    int sockfd;
    termios *orig_tios; // struct termios
    telnet_t *telnet; // Telnet state box
    int do_echo;
    int connected; // Connected to server
    pthread_t workerThread;
    int worker_running;
    data_recv_callback_t data_recv;
    void *data_recv_bundle;

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
    /**
     * @brief worker Worker thread for receiving data from server
     * @param data
     * @return
     */
    static void *worker(void *data);
};

#endif // QTELNET_H
