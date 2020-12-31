# Xarxes

Spaceship Royale is a 3 player online game made with C++ by Oscar Pons and David Tello. It is a third person shooter with spaceships where you have to be the last one standing.

Link to repo: https://github.com/ponspack9/Xarxes
Link to release: https://github.com/ponspack9/Xarxes/releases/tag/1.0

## Instructions
First you will have to have a server started and then there must be 3 players connected to start the game, once the game has started until it has finished no other players can connect to the server. Once you are killed you will be disconnected until only 1 player remains who will be sent a Win message and then disconnected after a few seconds and the game will finish making it able to start a new game.

A -> turn anticlockwise

D -> turn clockwise

DOWN ARROW -> Propel spaceship

LEFT ARROW -> shoot projectile

## Features
* [x] [Oscar] UDP virtual connection -> Achieved with some bugs
  * Most times when a client leaves the server instantly disconnects it and prompts a WSAECONNRESET message in the server side. And sometimes it seems not to notice and do expected behaviour that is waiting till the DISCONNECT_TIMEOUT_SECONDS is reached and then disconnects the client. With clients seems to have no effect the programmed timeout and leaves game instantly when server is disconnected, showing the same error mentioned above.
  * Possible reasons may be: ??
* [x] [Oscar] Accept 2 players or more -> Completely Achieved
* [x] [David] Handle join/leave events -> Completely Achieved
* [x] [Oscar]World State Replication -> Completely Achieved
    * ~~Liada padre a partir de mas de un jugador, o que el jugador 1 abandone y vuelva a entrar, algo con las ID esta pasando imagino~~
	* ~~NetworkId problems confirmed, now it's solved the previous issue (RegisterWithNetworkId). Remains a problem with delayed destroys and new instantiations: when the server has already delay destroyed a bullet and creates a new one, it assigns an index free for him but occupied on the clients, then an assert arises. Changing the index should be the solution but then the networkID is fuc*ed up, so I will keep thinking. I think I can confirm this bug because it shows up more frecuently when the intervals of sending the world state replication are longer, being easy to encounter this "data race".~~
	* ~~Thinking in having a confirmation from clients before unregistering a netobject from the server. That should work (Difficult&synch!)~~
	* Seen the slide that says to just ignore the client side and follow the server instructions T_T
* [x] [Oscar] Redundant sending of input packets -> Completely Achieved
* [x] [Oscar] Delivery Manager -> Partially achieved
	* Now there's only the basic functionality implemented: Server sends packets of replication with a sequence number and remember them, the client gets this packets, saves the sequence numbers and in intervals sends back the ack of them. This can be visualized in the GUI of each, and applying network losses, jitter do interfere with them and can be seen that packets gets accumulated or recieved in unnexpected order.
	* (Replication) Packets out of order are discarded, callback implementation is needed
	* (Input) Packets are resent with newer inputs till there's confirmation of them. TODO: send with replication packet, now is a specific inputAck packet.
* [ ] Client side prediction with server reconciliation -> Tried but not achieved
* [x] [David] Entity interpolation -> Completely Achieved
