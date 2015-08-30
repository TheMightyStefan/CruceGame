#include <cutter.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <irc.h>
#include <network.h>
#include <errno.h>

extern int currentRoom;

/**
 * Helper to open a local server socket.
 * Param port The port on which must be created the server.
 * Returns a sockfd.
 */
int openLocalhostSocket(int port) {
    int serverSockfd = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(serverSockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    cut_assert_true(serverSockfd >= 0, "Server socket opening failed");

    struct sockaddr_in serv_addr, cli_addr;
    bzero((char *)&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    int bnd = bind(serverSockfd, (struct sockaddr *)&serv_addr,
                   sizeof(serv_addr));
    cut_assert_operator_int(bnd, >=, 0, "Server bind failed");

    listen(serverSockfd, 5);
    socklen_t clilen = sizeof(cli_addr);

    int newsockfd = accept(serverSockfd, (struct sockaddr *)&cli_addr, &clilen);

    close(serverSockfd);

    cut_assert_operator_int(newsockfd, >=, 0, "Server accept failed");

    return newsockfd;
}

/**
 * Helper to connect to a local server socket.
 * Param port The port on which runs the server.
 * Returns none.
 *
 * This function assumes the use of sockfd private variable in the networking
 * module.
 */
int connectToLocalhostSocket(int port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    cut_assert_operator_int(sockfd, >=, 0,
                            "Could not connect to the server thread");

    struct hostent *server = gethostbyname("localhost");
    cut_assert_not_null(server, "No such host");

    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list[0],
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(port);

    int conn = connect(sockfd, (struct sockaddr *)&serv_addr,
                       sizeof(serv_addr));

    cut_assert_operator_int(conn, >=, 0,
                            "Error connecting to the server process");
    return sockfd;
}

void test_irc_connect()
{
    char expected_messages[3][4][513] = {
        {
            "PASS *\r\n",
            "NICK test_user\r\n",
            "USER test_user 8 * :test_user\r\n",
            "JOIN #cruce-devel\r\n"
        },
        {
            "PASS *\r\n",
            "NICK \r\n",
            "USER  8 * :\r\n",
            "JOIN #cruce-devel\r\n"
        },
        {
            "PASS *\r\n",
            "NICK test_user_\r\n",
            "USER test_user_ 8 * :test_user_\r\n",
            "JOIN #cruce-devel\r\n"
        }
    };

    char inputs[3][10] = {
        "test_user",
        "",
        "test_user_"
    };

    for (int i = 0; i < 3; i++) {
        int pid = cut_fork();
        if (pid == 0) {
            int server_sock = serverHelper();

            char buffer[513];
            for (int j = 0; j < 4; j++) {
                memset(buffer, 0, 513);
                cut_assert_operator_int(read(server_sock, buffer, 513), >=, 0);
                cut_assert_equal_string(expected_messages[i][j], buffer);
            }
            close(server_sock);

            exit(EXIT_SUCCESS);
        }

        irc_connect(inputs[i]);
    }
}

void test_irc_sendLobbyMessage()
{
    struct sockaddr_in test_server;
    initConnection(&test_server);

    char expected_messages[3][513] = {
        "PRIVMSG #cruce-devel test test test test\r\n",
        "PRIVMSG #cruce-devel \r\n",
        // Begins here
        "PRIVMSG #cruce-devel test test test test test "
        "test test test test test test test test test test test test test "
        "test test test test test test test test test test test test test "
        "test test test test test test test test test test test test test "
        "test test test test test test test test test test test test test "
        "test test test test test test test test test test test test test "
        "test test test test test test test test test test test test test "
        "test test test test test test test test test test test test  test "
        "test tes\r\n"
        // and ends here.
    };

    char inputs[3][514] = {
        "test test test test",
        "",
        // Begins here
        "test test test test test "
        "test test test test test test test test test test "
        "test test test test test test test test test test "
        "test test test test test test test test test test "
        "test test test test test test test test test test "
        "test test test test test test test test test test "
        "test test test test test test test test test test "
        "test test test test test test test test test test "
        "test test test test test test test test test test "
        "test test test test test test test test test test "
        "test test test test test test test"
        // and ends here.
    };

    // If test_parametersp[i] == 1 then
    // cut_assert_equal_int(irc_sendLobbyMessage(inputs[i]), 0) will be issued
    // otherwise cut_assert_not_equal_int(irc_sendLobbyMessage(inputs[i]), 0).
    int test_parameters[3] = {1, 1, 0};

    for (int i = 0; i < 3; i++) {
        int pid = cut_fork();
        if (pid == 0) {
            int server_sock = serverHelper();

            char buffer[513];
            memset(buffer, 0, 513);
            cut_assert_operator_int(read(server_sock, buffer, 513), >=, 0);
            cut_assert_equal_string(expected_messages[i], buffer);

            close(server_sock);
            exit(EXIT_SUCCESS);
        }

        int server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        cut_assert_operator_int(server_sock, >=, 0);

        cut_assert_operator_int(connect(server_sock,
                                        (struct sockaddr *)&test_server,
                                        sizeof(test_server)), >=, 0);

        if (test_parameters[i]) {
            cut_assert_equal_int(irc_sendLobbyMessage(inputs[i]), 0);
        } else {
            cut_assert_not_equal_int(irc_sendLobbyMessage(inputs[i]), 0);
        }
        close(server_sock);
    }
}

void test_irc_disconnect()
{
    struct sockaddr_in test_server;
    initConnection(&test_server);

    char expected_message[513] = "QUIT\r\n";

    int pid = cut_fork();
    if (pid == 0) {
        int server_sock = serverHelper();

        char buffer[513];
        memset(buffer, 0, 513);
        cut_assert_operator_int(read(server_sock, buffer, 513), >=, 0);
        cut_assert_equal_string(expected_message, buffer);

        close(server_sock);
        exit(EXIT_SUCCESS);
    }

    int server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    cut_assert_operator_int(connect(server_sock,
                                    (struct sockaddr *)&test_server,
                                    sizeof(test_server)), >=, 0);

    cut_assert_equal_int(irc_disconnect(), 0);
    close(server_sock);
}

void test_irc_joinRoom()
{
    struct sockaddr_in test_server;
    initConnection(&test_server);

    char expected_messages[4][513] = {
        "JOIN #cruce-devel001\r\n",
        "JOIN #cruce-devel000\r\n",
        "JOIN #cruce-devel999\r\n",
        "JOIN #cruce-devel1000\r\n"
    };

    int inputs[4] = {1, 0, 999, 1000};

    int test_parameters[4] = {1, 1, 1, 0};

    for (int i = 0; i < 4; i++) {
        int pid = cut_fork();
        if (pid == 0) {
            int server_sock = serverHelper();

            char buffer[513];
            memset(buffer, 0, 513);

            if (test_parameters[i]) {
                cut_assert_operator_int(read(server_sock, buffer, 513), >=, 0);
            } else {
                // If we try to connect to an invalid room (`#cruce-devel1000`,
                // for example) the server should not receive any message
                // from the client. Thus, `read` will return 0.
                cut_assert_equal_int(read(server_sock, buffer, 513), 0);
            }

            if (test_parameters[i]) {
                cut_assert_equal_string(expected_messages[i], buffer);
            }

            close(server_sock);
            exit(EXIT_SUCCESS);
        }

        int server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        cut_assert_operator_int(connect(server_sock,
                                        (struct sockaddr *)&test_server,
                                        sizeof(test_server)), >=, 0);

        if (test_parameters[i]) {
            cut_assert_equal_int(irc_joinRoom(inputs[i]), 0);
        } else {
            cut_assert_not_equal_int(irc_joinRoom(inputs[i]), 0);
        }
        close(server_sock);
    }
}

void test_irc_leaveRoom()
{
    struct sockaddr_in test_server;
    initConnection(&test_server);

    char expected_message[] = "PART #cruce-devel002\r\n";
    int currentRoomValues[] = {2, -1000};
    int test_parameters[] = {1, 0};

    for (int i = 0; i < 2; i++) {
        currentRoom = currentRoomValues[i];

        int pid = cut_fork();
        if (pid == 0) {
            int server_sock = serverHelper();

            char buffer[513];
            memset(buffer, 0, 513);

            if (test_parameters[i]) {
                cut_assert_operator_int(read(server_sock, buffer, 513), >=, 0);
            } else {
                // If we are on this branch it means we are testing the case
                // where `currentRoom` has an invalid value, thus `read` should
                // return `0`.
                cut_assert_equal_int(read(server_sock, buffer, 513), 0);
            }

            if (test_parameters[i]) {
                cut_assert_equal_string(expected_message, buffer);
            }

            close(server_sock);
            exit(EXIT_SUCCESS);
        }

        int server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        cut_assert_operator_int(connect(server_sock,
                                (struct sockaddr *)&test_server,
                                 sizeof(test_server)), >=, 0);

        if (test_parameters[i]) {
            cut_assert_equal_int(irc_leaveRoom(), 0);
        } else {
            cut_assert_not_equal_int(irc_leaveRoom(), 0);
        }
    }
}
