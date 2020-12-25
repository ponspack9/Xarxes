# Xarxes

GAME NAME is a 2 player online game made with C++ by Oscar Pons and David Tello, it is a tribute to hte game Asteroids but with a second player and the objective is to destroy the enemy ship while avoiding and destroying asteroids.

## Instructions


## Features
**feature, author, degree of completeness (tried but not achieved/achieved with some known bugs/completely achieved), result accomplished & known bugs/issues**
* [x] [Oscar] UDP virtual connection -> Achieved with some bugs
  * Most times when a client leaves the server instantly disconnects it and prompts a WSAECONNRESET message in the server side. And sometimes it seems not to notice and do expected behaviour that is waiting till the DISCONNECT_TIMEOUT_SECONDS is reached and then disconnects the client.
With clients seems to have no effect the programmed timeout and leaves game instantly when server is disconnected, showing the same error mentioned above.
* [x] [Oscar] Accept 2 players or more -> Completely achieved
* [ ] Handle join/leave events
* [ ] World State Replication
* [ ] Redundant sending of input packets
* [ ] Delivery Manager (successful deliveries / delivery failures on timeout -> resend current state)
* [ ] Client side prediction with server reconciliation
* [ ] Entity interpolation
