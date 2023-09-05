#ifndef _PORTICO_TIMER_
#define _PORTICO_TIMER_

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>

#include <time.h>

/**
 * @brief Structure to hold client data including address, socket, and timer.
 */
struct client_data
{
    sockaddr_in address; // Client's address information.
    int sockfd;          // Socket file descriptor for the client.
    util_timer *timer;   // Timer associated with the client.
};

/**
 * @brief Timer class to manage timers.
 */
class util_timer
{
public:
    util_timer() : prev(NULL), next(NULL) {}

public:
    time_t expire; // Expiration time for the timer.

    void (*cb_func)(client_data *); // Callback function associated with the timer.
    client_data *user_data;         // User data associated with the timer.
    util_timer *prev;               // Pointer to the previous timer.
    util_timer *next;               // Pointer to the next timer.
};

/**
 * @brief Class to manage a sorted timer list.
 */
class sort_timer_lst
{
public:
    sort_timer_lst();
    ~sort_timer_lst();

    void add_timer(util_timer *timer);
    void adjust_timer(util_timer *timer);
    void del_timer(util_timer *timer);
    void tick();

private:
    void add_timer(util_timer *timer, util_timer *lst_head);

    util_timer *head_; // Pointer to the head of the timer list.
    util_timer *tail_; // Pointer to the tail of the timer list.
};

/**
 * @brief Utility class for various operations.
 */
class Utils
{
public:
    Utils() {}
    ~Utils() {}

    /**
     * @brief Initialize the utility class with a specified time slot.
     * @param timeslot The time slot value.
     */
    void init(int timeslot);

    /**
     * @brief Set a file descriptor to non-blocking mode.
     * @param fd The file descriptor to set as non-blocking.
     * @return 0 on success, -1 on failure.
     */
    int setnonblocking(int fd);

    /**
     * @brief Add a file descriptor to the epoll event table for monitoring.
     * @param epollfd The epoll file descriptor.
     * @param fd The file descriptor to add.
     * @param one_shot Whether to use EPOLLONESHOT.
     * @param TRIGMode The trigger mode.
     */
    void addfd(int epollfd, int fd, bool one_shot, int TRIGMode);

    /**
     * @brief Signal handler for various signals.
     * @param sig The signal number.
     */
    static void sig_handler(int sig);

    /**
     * @brief Add a signal handler for a specific signal.
     * @param sig The signal number.
     * @param handler The signal handler function.
     * @param restart Whether to restart system calls on interruption.
     */
    void addsig(int sig, void(handler)(int), bool restart = true);

    /**
     * @brief Handle timer events and trigger SIGALRM signals.
     */
    void timer_handler();

    /**
     * @brief Show an error message and close the connection.
     * @param connfd The connection file descriptor.
     * @param info The error message.
     */
    void show_error(int connfd, const char *info);

public:
    static int *u_pipefd;       // Pointer to the pipe file descriptor.
    sort_timer_lst m_timer_lst; // Timer list for managing timers.
    static int u_epollfd;       // Epoll file descriptor for event monitoring.
    int m_TIMESLOT;             // Time slot value.
};

/**
 * @brief Callback function for timer events.
 * @param user_data User data associated with the timer.
 */
void cb_func(client_data *user_data);

#endif
