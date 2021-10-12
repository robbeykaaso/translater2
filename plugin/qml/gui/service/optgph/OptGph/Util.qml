pragma Singleton
import QtQuick 2.12

QtObject {
    function arrayToText(aArray){
        var txt = ""
        for (var i in aArray){
            if (txt !== "")
                txt += "、"
            txt += aArray[i]
        }
        return txt
    }
}
