#include "ssdp.h"
#include <QNetworkInterface>
#include "util.h"

namespace rea2 {

DiscoveryManager::DiscoveryManager(quint16 aReceivePort, QObject *parent)
    : QObject(parent)
{
    auto adr = QNetworkInterface::allAddresses();
    for (auto i : adr){
        if (i.protocol() != QAbstractSocket::NetworkLayerProtocol::IPv4Protocol)
            continue;
        auto sock_ = std::make_shared<QUdpSocket>(this);
        auto ok = sock_->bind(i, aReceivePort, QUdpSocket::ShareAddress);

        if (!ok){
            //printf("Bind Error\n");
            continue;
        }

        ok = sock_->joinMulticastGroup(groupAddress);
        if (!ok) {
            //printf("Join Multicast Group Failed\n");
            continue;
        }

        connect(sock_.get(), SIGNAL( readyRead()), this, SLOT(ReadPending()));

        socks_.push_back(sock_);
        auto lg = "Bind Address: " + i.toString().toStdString() + " success!";
        std::cout << lg << std::endl;
    }


}

//https://blog.csdn.net/swanabin/article/details/52024800
void DiscoveryManager::StartDiscovery(const std::string& aST)
{
    auto msg = "M-SEARCH * HTTP/1.1\r\n"        \
                       "HOST: 239.255.255.250:1900\r\n" \
                       "MAN: \"ssdp:discover\"\r\n" \
                       "ST: " + aST + "\r\n" \
                       //"MX: 1\r\n"
                     // "ST: upnp:rootdevice\r\n" \
                      // "USN: uuid:7e22e310-2f43-11e8-a588-6c5ab5f9ef8b::upnp:rootdevice"
              "\r\n";
    QByteArray message(msg.c_str());
    for (auto i : socks_){
        auto writeOk = i->writeDatagram(message.data(), groupAddress, boardcast_port);
        if (writeOk == -1) {
            printf("Writing Datagram failed\n");
            return;
        }
    }
}

/*void DiscoveryManager::SendAlive()
{
    QByteArray message(
        "NOTIFY * HTTP/1.1\r\n" \
        "Host: 239.255.255.250:1900\r\n" \
        "Location: http://192.168.1.147:8000/\r\n" \
        "NTS: ssdp:alive\r\n" \
        "Cache-Control: max-age=30\r\n" \
        "Server: UPnP/1.0 DLNADOC/1.50 Allwinnertech/0.1.0\r\n" \
        "USN: uuid:7e22e310-2f43-11e8-a588-6c5ab5f9ef8c::urn:schemas-tencent-com:service:QPlay:2\r\n" \
        "NT: urn:schemas-tencent-com:service:QPlay:2\r\n" \
        "\r\n");
    auto writeOk = sock_->writeDatagram(message.data(), groupAddress, boardcast_port);
    if (writeOk == -1) {
        printf("Writing Datagram failed\n");
        return;
    }
}*/
//#include "glog/logging.h"
void DiscoveryManager::ReadPending()
{
    for (auto i : socks_){
        if (i->hasPendingDatagrams()) {
            QByteArray reply;
            reply.resize(i->pendingDatagramSize());
            i->readDatagram(reply.data(), reply.size());
            //LOG(INFO) << "hello";
            QString tmp;
            auto content = tmp.prepend(reply).split("\r\n");
            QString ip, port, id;
            for (auto c : content)
                if (c.indexOf("Location:") >= 0){
                    auto loc = c.split(":");
                    ip = loc[1].trimmed();
                    port = loc[2].trimmed();
                    //FoundServer(loc[1], loc[2]);
                }else if (c.indexOf("USN:") >= 0){
                    auto loc = c.split(":");
                    id = loc[1].trimmed();
                }
            FoundServer(ip, port, id);
        }
    }
}

}
