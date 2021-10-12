import QtMultimedia 5.13
import QtQuick 2.12

Item{
    property string name

    MediaPlayer{
        id: md
        autoPlay: true
    }

    VideoOutput{
        anchors.fill: parent
        source: md
    }

    MouseArea{
        anchors.fill: parent
        onPressed:{
            //console.log("hi")
            if (md.playbackState == MediaPlayer.PlayingState)
                md.pause()
            else
                md.play()
        }
    }
    Component.onCompleted: {
        Pipelines().add(function(aInput){
            //md.source = ""
            md.source = "file:" + aInput.data()
        }, {name: "updateMideaAttr_" + name})
    }
}
