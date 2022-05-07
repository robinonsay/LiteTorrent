import time
import subprocess
import signal

from mininet.cli import CLI
from mininet.net import Mininet
from mininet.link import TCLink
from mininet.topo import Topo
from mininet.log import setLogLevel

class AssignmentNetworks(Topo):
    def __init__(self, **opts):
        Topo.__init__(self, **opts)
    #Set up Network Here

    def build(self):
        """
        Builds custom topology
        """
        hosts = []
        num_hosts = 10
        for i in range(1, num_hosts+1):
            hosts.append(self.addHost(f'h{i}'))
        # Add Links
        switch = self.addSwitch('s0')
        for host in hosts:
            self.addLink(host, switch)

if __name__ == '__main__':
    setLogLevel( 'info' )

    # Create data network
    topo = AssignmentNetworks()
    net = Mininet(topo=topo, link=TCLink, autoSetMacs=True,
           autoStaticArp=True)
    # Run network
    net.start()
    CLI(net)
    net.stop()

