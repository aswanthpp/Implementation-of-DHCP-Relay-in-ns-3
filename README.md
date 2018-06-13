# Extension ​of DHCPv4​​ implementation​ ​in​ ​ns-3​ ​to​ ​support​ ​DHCP​ ​(or​ ​BOOTP)​ ​Relay
## Course Code : CO300
## Assignment :  #14 

### Overview

The Dynamic Host Configuration Protocol (DHCP) provides configuration parameters to Internet hosts. DHCP consists of two components: a protocol for delivering host-specific configuration parameters from a DHCP server to a host and a mechanism for allocation of network addresses to hosts. However, it is not feasible to have a server on each subnet.

Relay agents are used to forward requests and replies between clients and servers when they are not on the same physical subnet. Relay agent forwarding is distinct from the normal forwarding of an IP router, where IP datagrams are switched between networks somewhat transparently. By contrast, relay agents receive DHCP messages and then generate a new DHCP message to send out on another interface. DHCP is designed to use the same relay agent behavior as specified in the BOOTP protocol specification. 

### Working
* Below is an example for a network topology consisting of a relay agent :



<p align="center">
<img src="https://user-images.githubusercontent.com/19391965/33467863-e3b40746-d67d-11e7-8181-87a32d2197ec.png"/>
</p>

* An incoming client(s) (source IP address of 0.0.0.0) first sends a **DHCP Discover message** within a UDP packet to port 67 as a broadcast (destination IP address of 255.255.255.255).
* The relay agent receives this packet on the port in the client subnet and forwards to the server via the port in the server side.
* The relay agent (which maintains the server's IP address) unicasts this packet to the DHCP server.
* A DHCP server receiving a DHCP discover message responds to the client with a **DHCP Offer message** that is unicast to the relay agent.
* Each server offer message contains the transaction ID of the received discover message, the proposed IP address for the client, the network mask, and an IP address lease time.
* On receiving the offer message from the DHCP server, the relay agent broadcasts it to all node(s) on the client side subnet using the IP broadcast address of 255.255.255.255.
* The newly arriving client will respond to the offer with a  **DHCP Request message**.
* The relay agent once again forwards this to the DHCP server.
* The server responds to the DHCP request message with a **DHCP ACK message**.
* The relay agent sends the ACK to the client confirming the requested parameters.
* Once the client receives the DHCP ACK, the interaction is complete and the client can use the DHCP-allocated IP address for the lease duration (which can be renewed).


### To run the code

First we need to download and use the development version of ns-3.
If you have successfully installed mercurial, you can get copy of the development version with the following command :

`hg clone http://code.nsnam.org/ns-3-allinone`

`cd ns-3-allinone && ./download.py`

To build ns-3 downloaded, open the terminal to `ns-3-allinone` directory and enter the following command :

`./build.py`

Now clone the repository to your local machine.

`git clone https://github.com/aswanthpp/Implementation-of-DHCP-Relay-in-ns-3.git`

Replace the contents in ns-3-dev directory with the cloned directory files. Then configure examples and tests in ns-3 using :

`./waf configure --enable-examples --enable-tests`

An example program for DHCP Relay has been provided in :

`src/internet-apps/examples/dhcp-example-relay.cc`

Once ns-3 has been built and examples,tests are enabled, we can run the example file as :

`./waf --run "src/internet-apps/examples/dhcp-example-relay"`

To enable logging

`./waf --run "src/internet-apps/examples/dhcp-example-relay --verbose=True"`

To enable trace files

`./waf --run "src/internet-apps/examples/dhcp-example-relay --tracing=True"`

### Future Work

The existing DHCPv4 code limits only one server for each relay agent in a network. We have prepared a document listing the changes required to make the code scalable so as to let the server assign addresses to different subnets. The proposed changes can be found <a href="https://github.com/aswanthpp/Implementation-of-DHCP-Relay-in-ns-3/wiki/Future-Work" target="blank">here</a>.

### References


[1]  RFC​ ​ 2131:​ ​ Dynamic​ ​ Host​ ​ Configuration​ ​ Protocol​ ​ (  <a href="https://tools.ietf.org/pdf/rfc2131.pdf" target="blank">https://tools.ietf.org/pdf/rfc2131.pdf</a> )

[2]  DHCP​ ​ module​ ​ for​ ​ IPv4​ ​ in​ ​ ns-3​ ​ ( src/internet-apps/model )
