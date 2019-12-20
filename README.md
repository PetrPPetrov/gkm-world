# gkm-world
Open Source Massive Multiplayer Online Battle Game

## TODO list
* avoid InitializePositionInternalAnswer packet from Node->Balancer, send it directly Node->Proxy
* rename BalanceTree:split() to BalanceTree:staticSplit()
* introduce parameters for proxy server IP address in balancer server
* introduce parameters for proxy server IP address in node server
* use successful double action policy for packet delivery
* prepare visible users list on node server for quick fetching