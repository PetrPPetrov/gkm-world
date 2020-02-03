# gkm-world
Open Source Massive Multiplayer Online Battle Game

# Dependencies
* Boost library 1.60 or higher (Boost.Asio for network)
* QT library version 5.x (for Balancer Monitor)
* bgfx library - https://github.com/bkaradzic/bgfx.git (for Windows client)
* vcpkg - https://github.com/microsoft/vcpkg.git with installed the following packets:
  * vcpkg install libjpeg-turbo --triplet x64-windows-static
  * vcpkg install tinyobjloader --triplet x64-windows-static

## TODO list
* avoid InitializePositionInternalAnswer packet from Node->Balancer, send it directly Node->Proxy
* use successful double action policy for packet delivery
* prepare visible users list on node server for quick fetching
