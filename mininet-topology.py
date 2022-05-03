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
    trackerCmd = "./tracker peers/peerlist.txt test-files/test3.jpg trrnt-files/torrent-file.txt logs/log.txt" 
    # Run network
    net.start()
    h1 = net.get('h1')
    h2 = net.get('h2')
    h3 = net.get('h3')
    h4 = net.get('h4')
    peer2Cmd = f"./peer {h2.IP()} {h1.IP()} test-files/test3.jpg ownd-chunks/h1.txt output/out2.jpg logs/log2.txt"
    peer3Cmd = f"./peer {h3.IP()} {h1.IP()} test-files/test3.jpg ownd-chunks/h2.txt output/out3.jpg logs/log3.txt"
    peer4Cmd = f"./peer {h4.IP()} {h1.IP()} test-files/test3.jpg ownd-chunks/h3.txt output/out4.jpg logs/log4.txt"
    h1p = h1.popen(trackerCmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    time.sleep(1)
    h2p = h2.popen(peer2Cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    h3p = h3.popen(peer3Cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    h4p = h4.popen(peer4Cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    time.sleep(30)
    h1p.send_signal(signal.SIGINT)
    h2p.send_signal(signal.SIGINT)
    h3p.send_signal(signal.SIGINT)
    h4p.send_signal(signal.SIGINT)
    print("------------------h1------------------")
    print(h1p.communicate()[0].decode('ascii'))
    print("------------------h2------------------")
    print(h2p.communicate()[0].decode('ascii'))
    print("------------------h3------------------")
    print(h3p.communicate()[0].decode('ascii'))
    print("------------------h4------------------")
    print(h4p.communicate()[0].decode('ascii'))
    h1p.kill()
    h2p.kill()
    h3p.kill()
    h4p.kill()
#    CLI(net)
    net.stop()

