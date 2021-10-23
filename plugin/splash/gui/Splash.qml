import QtQuick 2.12
import QtQuick.Window 2.12

Window {
    id: splash
    color: "transparent"
    title: "Splash Window"
    modality: Qt.ApplicationModal
    flags: Qt.SplashScreen | Qt.WindowStaysOnTopHint
    x: (Screen.width - splashImage.width) / 2
    y: (Screen.height - splashImage.height) / 2
    width: splashImage.width
    height: splashImage.height
    visible: true

    //property alias timer: timer

    Image {
        id: splashImage
        source: "file:nwlan_ui/gui/assets/splash.png"
        Text{
            x: parent.width - width - 20
            y: 240
            wrapMode: Text.WordWrap
            text: "loading..."
        }
    }
   /* Text{
        id: textCtrl
        width: contentWidth
        height: contentHeight
        anchors{left: splashImage.left; bottom: splashImage.bottom}
        font{pointSize: 30}
    }

    Timer {
        id: timer
        interval: 1000; running: false; repeat: false
        onTriggered: {
            splash.visible = false;
        }
    }

    Loader{
        id: loader
        asynchronous: true
        source: "qrc:/html_main.qml"
        active: false
        onLoaded: {
            timer.start()
        }
    }*/

    Timer {
        id: timer
        interval: 1000; running: false; repeat: false
        onTriggered: {
            splash.close()
        }
    }

    Component.onCompleted: {
        timer.start()
    }
}
