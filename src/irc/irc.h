/**
 * @file This module implements all the irc operations.
 *       It is the only module which has access to the network module.
 *
 *       The concepts used by this module are:
 *          - lobby       : an irc channel that is joined automatically when
 *                          the game starts, used as a chat.
 *          - room        : an irc channel where a game is played.
 *                          Any user can join at most one room.
 *          - room status : it is represented by the channel's topic,
 *                          that can be PLAYING or WAITING
 *          - invitation  : any user can be invited to join a room.
 *
 *       This module implements an event driven interface for the events
 *       JOIN, QUIT, NOTICE, INVITE, PRIVMSG.
 *
 * TODO: add handler register/unregister methods.
 */

/**
 * @brief Connect to the irc server and join the looby.
 *
 * @return 0 on success, other value on failure.
 */
void irc_connect();

/**
 * @brief Join a room. AT MOST ONE ROOM CAN BE JOINED AT ANY TIME.
 *
 * @return 0 on success, other value on failure.
 */
void irc_joinRoom();

/**
 * @brief Leave the room.
 *
 * @return 0 on success, other value on failure.
 */
void irc_leaveRoom();

/**
 * @brief End the connection to the irc server, doing the necessary teardown
 *        operations.
 *
 * @return 0 on success, other value on failure.
 */
void irc_disconnect();

/**
 * @brief Create a new room and join it.
 *
 * @return Room id on success, negative value on failure.
 */
int irc_createRoom(char *channelName);

/**
 * @brief Toggle the status of the current room.
 *
 * @return 0 on success, other value on failure.
 */
int irc_toggleRoomStatus();

/**
 * @brief Get all users from the lobby or from room.
 *
 * @param isRoom If 1, the users from the current room are returned.
 *               Otherwise, the users from the lobby are returned.
 *
 * @return Pointer to a string containing the nicknames of all users in the
 *         lobby or in the room. The nicknames are separated by a space.
 *         MUST BE FREED.
 *         On failure, NULL is returned.
 */
char *irc_getNames(int isRoom);

/**
 * @brief List all available rooms. Cannot be used if already joined in a room.
 *
 * @return A string containing all the available rooms on success,
 *         NULL on failure.
 */
char *irc_getAvailableRooms();

/**
 * @brief Invite to room. Cannot be used if not joined in a room.
 *
 * @param nickname The nickname of the user to be invited.
 *
 * @return 0 on success, other value on failure.
 */
int irc_invite(char *nickname);

/**
 * @brief Send an IRC message.
 *
 * @param message The message to be sent. Must be NUL terminated.
 * @param toRoom If 0, the message is sent to the lobby.
 *               Otherwise, it is sent to the joined room.
 *
 * @return 0 on success, other value on failure.
 */
int irc_sendMessage(char *message, int toRoom);

/**
 * @brief Read a new message and handle it.
 */
void irc_handleNextMessage();
