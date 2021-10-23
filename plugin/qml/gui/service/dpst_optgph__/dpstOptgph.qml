import QtQuick 2.12
import QtQuick.Controls 2.12

Column{
    Button{
        text: Pipelines().tr("layout") + "(Shift+A)"
        onClicked: {
            Pipelines().run("enableLayout", "")
        }
    }
    Button{
        text: Pipelines().tr("apply")
        onClicked: {
            Pipelines().run("applyOperatorGraph", "", "", {remote: true, boardcast: true})
        }
    }
}
