# Copyright 2012 James McCauley
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
This component is for use with the OpenFlow tutorial.

It acts as a simple hub, but can be modified to act like an L2
learning switch.

It's roughly similar to the one Brandon Heller did for NOX.
"""

from pox.core import core
import pox.openflow.libopenflow_01 as of
import pox.lib.packet as pkt # POX convention
from pox.lib.packet.ethernet import ethernet

log = core.getLogger()



class Tutorial (object):
  """
  A Tutorial object is created for each switch that connects.
  A Connection object for that switch is passed to the __init__ function.
  """
  def __init__ (self, connection):
    # Keep track of the connection to the switch so that we can
    # send it messages!
    self.connection = connection

    # This binds our PacketIn event listener
    connection.addListeners(self)

    # Use this table to keep track of which ethernet address is on
    # which switch port (keys are MACs, values are ports).
    self.mac_to_port = {}

    fd = open("fw.conf", "r")

    # Firewall rules
    self.rules = {
            "ip_source": None,
            "ip_dest": None,
            "port": None,
            "protocol": None
    }

    for line in fd:
        if (line.isspace()) :
            continue
        words = line.rsplit()
        if (words[0] == "ip_source:") :
            self.rules["ip_source"] = words[1]
        elif (words[0] == "ip_dest:") :
            self.rules["ip_dest"] = words[1]
        elif (words[0] == "port:") :
            self.rules["port"] = int(words[1])
        elif (words[0] == "protocol:") :
            self.rules["protocol"] = words[1]


  def resend_packet (self, packet_in, out_port):
    """
    Instructs the switch to resend a packet that it had sent to us.
    "packet_in" is the ofp_packet_in object the switch had sent to the
    controller due to a table-miss.
    """
    msg = of.ofp_packet_out()
    msg.data = packet_in

    # Add an action to send to the specified port
    action = of.ofp_action_output(port = out_port)
    msg.actions.append(action)

    # Send message to switch
    self.connection.send(msg)


  def act_like_hub (self, packet, packet_in):
    """
    Implement hub-like behavior -- send all packets to all ports besides
    the input port.
    """

    # We want to output to all ports -- we do that using the special
    # OFPP_ALL port as the output port.  (We could have also used
    # OFPP_FLOOD.)

    # log.debug('Printing mvars of packet_in:')
    # print dir(packet_in)
    self.resend_packet(packet_in, of.OFPP_ALL)

    # Note that if we didn't get a valid buffer_id, a slightly better
    # implementation would check that we got the full data before
    # sending it (len(packet_in.data) should be == packet_in.total_len)).


  def act_like_switch (self, packet, packet_in):
    """
    Implement switch-like behavior.
    """

    # Here's some psuedocode to start you off implementing a learning
    # switch.  You'll need to rewrite it as real Python code.

    # Learn the port for the source MAC
    # log.debug('Printing mvars of packet_in:')
    # print dir(packet_in)
    # log.debug('Printing mvars of packet:')
    # print dir(packet)
    # log.debug('Printing TCP and UDP codes: ' + str(pkt.ipv4.TCP_PROTOCOL) + ", " + str(pkt.ipv4.UDP_PROTOCOL))
    # log.debug('packet.type: ' + str(packet.type))
    # log.debug('packet.find(tcp): ' + str(packet.find('tcp')))
    # log.debug('Got port for MAC Address ' + str(type(packet.dst)))
    self.mac_to_port[packet.src] = packet_in.in_port

    dest_mac = packet.dst

    if self.mac_to_port.get(dest_mac) is not None:
      # Send packet out the associated port
      #self.resend_packet(packet_in, self.mac_to_port[dest_mac])

      # Once you have the above working, try pushing a flow entry
      # instead of resending the packet (comment out the above and
      # uncomment and complete the below.)

      log.debug("Installing flow! Source port: " + str(packet_in.in_port) + ", destination port: " + str(self.mac_to_port[dest_mac]))
      # Maybe the log statement should have source/destination/port?

      msg = of.ofp_flow_mod()

      # Set fields to match received packet
      m = of.ofp_match()
      m.in_port = packet_in.in_port
      # log.debug('packet_in.in_port: ' + str(packet_in.in_port))
      m.dl_src = packet.src
      # log.debug('packet.src: ' + str(packet.src))
      m.dl_dst = packet.dst
      # log.debug('packet.dst: ' + str(packet.dst))
      if packet.type == ethernet.IP_TYPE:
          # We need this L2 field not wildcarded to be able to
          # match L3 fields nw_src, nw_dst and nw_proto
          m.dl_type = 0x0800
          ip_packet = packet.payload
          m.nw_src = ip_packet.srcip
          m.nw_dst = ip_packet.dstip
          if (packet.find('tcp') is not None):
              m.nw_proto = 6
          elif (packet.find('udp') is not None):
              m.nw_proto = 17
          # log.debug('ip_packet.srcip: ' + str(ip_packet.srcip))
          # log.debug('ip_packet.dstip: ' + str(ip_packet.dstip))
          # log.debug('Printing mvars of ip_packet:')
          # print dir(ip_packet)

      msg.match = m

      # Now do checks for firewall:
      # Note that you MUST define at least one Firewall rule!
      firewall_check = True

      if (self.rules["port"] is not None and self.rules["port"] != packet_in.in_port):
          log.debug('Difference found in port: ' + str(self.rules["port"]) + ", " + str(packet_in.in_port))
          firewall_check = False
      if (self.rules["protocol"] is not None):
          if (self.rules["protocol"] == "TCP" and packet.find('tcp') is None):
              log.debug('Difference found in protocol: ' + str(packet.find('tcp')))
              firewall_check = False
          elif (self.rules["protocol"] == "UDP" and packet.find('udp') is None):
              log.debug('Difference found in protocol: ' + str(packet.find('udp')))
              log.debug('Difference found in protocol')
              firewall_check = False
      if (self.rules["ip_source"] is not None and self.rules["ip_source"] != m.nw_src):
          log.debug('Difference found in source IP: ' + str(self.rules["ip_source"]) + ", " + str(m.nw_src))
          firewall_check = False
      if (self.rules["ip_dest"] is not None and self.rules["ip_dest"] != m.nw_dst):
          log.debug('Difference found in source IP: ' + str(self.rules["ip_dest"]) + ", " + str(m.nw_dst))
          firewall_check = False


      # Set other fields of flow_mod (timeouts? buffer_id?)
      # We decide on not setting any of those fields, for they are unecessary
      #
      # Add an output action, and send -- similar to resend_packet()
      if (firewall_check == False):
          log.debug('This packet is not to be dropped')
          action = of.ofp_action_output(port = self.mac_to_port[dest_mac])
          msg.actions.append(action)
      else:
          log.debug('PACKETS WILL BE DROPPED')

      self.connection.send(msg)
      log.debug('Message sent')

    else:
      # Flood the packet out everything but the input port
      # This part looks familiar, right?
      self.resend_packet(packet_in, of.OFPP_ALL)



  def _handle_PacketIn (self, event):
    """
    Handles packet in messages from the switch.
    """

    packet = event.parsed # This is the parsed packet data.
    if not packet.parsed:
      log.warning("Ignoring incomplete packet")
      return

    packet_in = event.ofp # The actual ofp_packet_in message.

    # Comment out the following line and uncomment the one after
    # when starting the exercise.
    #self.act_like_hub(packet, packet_in)
    self.act_like_switch(packet, packet_in)



def launch ():
  """
  Starts the component
  """
  def start_switch (event):
    log.debug("Controlling %s" % (event.connection,))
    Tutorial(event.connection)
  core.openflow.addListenerByName("ConnectionUp", start_switch)
