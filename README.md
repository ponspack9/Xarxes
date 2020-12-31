# Xarxes

Spaceship Royale is a 3 player online game made with C++ by Oscar Pons and David Tello.
It is a third person shooter with spaceships where you have to be the last one standing.

## Instructions
First you will have to have a server started and then there must be 3 players connected to start the game, once the game has started until it has finished no other players can connect to the server. Once you are killed you will be disconnected until only 1 player remains who will be sent a Win message and then disconnected after a few seconds and the game will finish making it able to start a new game.

## Features
**feature, author, degree of completeness (tried but not achieved/achieved with some known bugs/completely achieved), result accomplished & known bugs/issues**
* [x] [Oscar] UDP virtual connection -> Achieved with some bugs
  * Most times when a client leaves the server instantly disconnects it and prompts a WSAECONNRESET message in the server side. And sometimes it seems not to notice and do expected behaviour that is waiting till the DISCONNECT_TIMEOUT_SECONDS is reached and then disconnects the client.
With clients seems to have no effect the programmed timeout and leaves game instantly when server is disconnected, showing the same error mentioned above.
* [x] [Oscar] Accept 2 players or more -> Completely achieved
* [ ] Handle join/leave events -> Achieved with some bugs
* [ ] World State Replication
* [ ] Redundant sending of input packets
* [ ] Delivery Manager (successful deliveries / delivery failures on timeout -> resend current state)
* [ ] Client side prediction with server reconciliation
* [ ] Entity interpolation
