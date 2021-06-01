# What is Gkm-World?

Gkm-World is open-source massive multiplayer online game engine.

# Gkm-World generic architecture

Gkm-World uses UDP protocol at all for a fast network communication.

Gkm-World consists from servers and clients. Only client for Windows is available now.
Windows client uses BGFX framework.

Gkm-World server part consists of several servers:

* Proxy server
* Node server
* Balancer server

All these servers are located in a local Gkm-World server network.

Proxy server is a single server which is visible from the public Internet and a local Gkm-World server network.
Do not confuse Gkm-World proxy server with a generic web http-caching server (web proxy).
The main goal of Gkm-World proxy server is to send UDP packets from clients to a corresponding node server
and vice versa. We assume that a local Gkm-World server network will have only one instance of proxy server.

Node server performs main game computation logic, like collision detection, updating game units coordinates.
As computation resources as limited each node server responds only for its zone.
It is possible to have many instances of node servers a local Gkm-World server network.
A node server notifies its neighbor node servers about movement of game units when such game units
are close enough to zone borders. Neighbor node servers could take responsibility of gaming computation
for some units.

Balancer server manages all node servers, changes their responsibility zones.
Balancer server could spawns new node servers and stop the existing node servers,
it could split the existing responsibility zones and merge the neighbor responsibility zones.
We assume that a local Gkm-World server network will have only one instance of balancer server.

Balancer monitor is GUI application which connects to balancer server and displays the current responsibility zones.
Also it is possible to manually split or merge responsibility zones by using balancer monitor.

# Proxy server

Proxy server is single-threaded network console application.

# Node server

Node server is two-threaded network console application.
One thread is for network communication, another thread is for game logic computation.

# Balancer server

Balancer server is single-threaded network console application.

# Windows client

Windows client is two-threaded graphics & network Win32 GUI application.
One thread is for network communication,
another thread is for GUI interaction and 3D visualization.

# Dependencies
* Boost library 1.60 or higher (Boost.Asio for network)
* QT library version 5.x (for Balancer Monitor)
* BGFX library - https://github.com/bkaradzic/bgfx.git (for Windows client)
* vcpkg - https://github.com/microsoft/vcpkg.git with installed the following packages:
  * vcpkg install libjpeg-turbo --triplet x64-windows-static (for Windows client)
  * vcpkg install boost --triplet x64-windows-static (for Windows client)
  * vcpkg install tinyobjloader --triplet x64-windows (for resource packer)

# TODO list
* implement Windows client
* avoid InitializePositionInternalAnswer packet from Node->Balancer, send it directly Node->Proxy
* use successful double action policy for packet delivery
* prepare visible users list on node server for quick fetching
* implement some basic version of Mesh Builder
