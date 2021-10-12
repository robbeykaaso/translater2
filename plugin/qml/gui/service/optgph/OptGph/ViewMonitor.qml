import QtQuick 2.12

Item{
    Component.onCompleted: {
        Pipelines().find("ideVisibleChanged").nextF(function(aInput){
            var dt = aInput.data()
            parent.val = []
            var mdl = []
            for (var i in dt)
                if (dt[i] && (i.endsWith("_ide_image") || i.endsWith("_ide_dpst_anno"))){
                    parent.val.push(i)
                    mdl.push(i.split("_")[2].split("gridder")[1])
                }
            var idx = parent.combo.currentIndex
            parent.combo.model = mdl
            parent.combo.currentIndex = idx
        })
        Pipelines().run("ideVisibleChanged", {})
    }
}
