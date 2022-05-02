import time
import subprocess

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
    trackerCmd = "./tracker peers/peerlist.txt test-files/test3.jpg trrnt-files/torrent-file.txt logs/log.txt" 

    # Run network
    net.start()
#    h1 = net.get('h1')
#    h2 = net.get('h2')
#    h1.cmdPrint(trackerCmd)
#     h1p = h1.popen(, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
#     time.sleep(3)
#     h1p.kill()
#     h1out = h1p.communicate()[0]
#     print(h1out.decode('ascii'))
    CLI(net)
    net.stop()

