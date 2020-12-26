# Xarxes

GAME NAME is a 2 player online game made with C++ by Oscar Pons and David Tello, it is a tribute to hte game Asteroids but with a second player and the objective is to destroy the enemy ship while avoiding and destroying asteroids.

## Instructions


## Features
**feature, author, degree of completeness (tried but not achieved/achieved with some known bugs/completely achieved), result accomplished & known bugs/issues**
* [x] [Oscar] UDP virtual connection -> Achieved with some bugs
  * Most times when a client leaves the server instantly disconnects it and prompts a WSAECONNRESET message in the server side. And sometimes it seems not to notice and do expected behaviour that is waiting till the DISCONNECT_TIMEOUT_SECONDS is reached and then disconnects the client. With clients seems to have no effect the programmed timeout and leaves game instantly when server is disconnected, showing the same error mentioned above.
  * Possible reasons may be: ??
* [ ] Accept 2 players or more -> Nope (World replication bug)
* [ ] Handle join/leave events
* [x] [Oscar]World State Replication -> Achieved with some known bugs
    * ~~Liada padre a partir de mas de un jugador, o que el jugador 1 abandone y vuelva a entrar, algo con las ID esta pasando imagino~~
	* NetworkId problems confirmed, now it's solved the previous issue (RegisterWithNetworkId). Remains a problem with delayed destroys and new instantiations: when the server has already delay destroyed a bullet and creates a new one, it assigns an index free for him but occupied on the clients, then an assert arises. Changing the index should be the solution but then the networkID is fuc*ed up, so I will keep thinking. I think I can confirm this bug because it shows up more frecuently when the intervals of sending the world state replication are longer, being easy to encounter this "data race".
	* [Update] Thinking in having a confirmation from clients before unregistering a netobject from the server. That should work
* [ ] Redundant sending of input packets
* [ ] Delivery Manager (successful deliveries / delivery failures on timeout -> resend current state)
* [ ] Client side prediction with server reconciliation
* [ ] Entity interpolation


## Project statement

* [ ] The submission deadline is December 31st at 23:59:59.
* [ ] To deliver the exercise, create a release in GitHub and upload the link to the release into the CITM virtual campus: Multiplayer game C++.
* [ ] In the ZIP file, include a folder with the solution and the source code.
* [ ] If the release cannot be properly executed, the exercise won’t be accepted.
* [ ] Do not disable the UI to simulate real world conditions. Will be key for the teacher to test the effectiveness of the implemented techniques.

Implement a simple 2D game prototype using the skeleton of the engine provided at class. Something similar to the initial spaceship wars seen in the example will do, but a bit extended (i.e. introduce some online game mechanics). Other examples if you decide to change the genre could be games such as Pong, collaborative space invaders, 2D racing games, 2D RPG-like
with overhead camera, etc.
Even if a polished result and a good idea for a multiplayer game will be taken into account, there is more interest in developing the techniques explained during the lab sessions in order to mitigate the impact of the network issues (to handle latency, jitter, and packet loss).

### Documentation requirements
The documentation will be presented in a markdown or text file (README.md or README.txt) provided in the root folder of the release. It will contain the following information:
* [x] Names of the group members
* [ ] Game name and description
* [ ] Gameplay tutorial / instructions
* [ ] List of implemented features/techniques and authors of each one
    * [ ] It is expected that everybody implements some of the techniquesexplained in the lab sessions (i.e. everybody has to code).
    * [ ] For each feature in the list, describe the degree of completeness reached: tried but not achieved / achieved with some known bugs /completely achieved.
    * [ ] For each feature in the list explain also the result accomplished and the known bugs or issues found. Flawless implementations that execute with no issues and solve the problem addressed perfectly will have short explanations. Otherwise, you are meant to be verbose on the identified bugs and the reason why you think they happen. 
    * [ ] Be honest! Your code and released binaries are there at GitHub.