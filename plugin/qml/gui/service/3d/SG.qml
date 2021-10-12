import Qt3D.Core 2.9
import Qt3D.Render 2.9
import Qt3D.Extras 2.9
import Qt3D.Input 2.0
import QtQuick.Scene3D 2.0
import QtQuick 2.0
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import CustomCameraController 1.0

Column{
    id: root
    property string name
    property var vw
    visible: false
    Row{
        width: parent.width
        height: 20
        Button{
            width: 60
            height: parent.height
            text: Pipelines().tr("fitView")
            onClicked: {
                if (vw)
                    vw.fitView()
            }
        }
    }
    Component{
        id: scene3d
        Scene3D {
            id: scrt
            width: root.width
            height: root.height - 20
            focus: true
            aspects: ["input", "logic"]
            cameraAspectRatioMode: Scene3D.AutomaticAspectRatio

            function loadModel(aSource){
                if (!aSource.startsWith(".")){
                    scrt.visible = true
                    mesh.source = ""
                    mesh.source = "file:" + aSource
                }
                else
                    scrt.visible = false
            }

            function fitView(){
                var mn = mesh.geometry.minExtent,
                    mx = mesh.geometry.maxExtent
                if (mn.x < mx.x || mn.y < mx.y || mn.z < mx.z){
                    camera.viewCenter = Qt.vector3d((mn.x + mx.x) * 0.5, (mn.y + mx.y) * 0.5, (mn.z + mx.z) * 0.5)
                    var dis = Math.max(mx.x - mn.x, mx.y - mn.y, mx.z - mn.z)
                    camera.position = Qt.vector3d( (mn.x + mx.x) * 0.5, (mn.y + mx.y) * 0.5, mx.z + dis * 2)
                    camera.upVector = Qt.vector3d( 0.0, 1.0, 0.0 )
                    camera.farPlane = mx.z + dis * 3
                    ctrl.linearSpeed = dis * 2
                    return true
                }
                return false
            }

            Entity {
                id: sceneRoot

                Camera {
                    id: camera
                    projectionType: CameraLens.PerspectiveProjection
                    fieldOfView: 45
                    nearPlane : 0.1
                    farPlane : 1000.0
                    position: Qt.vector3d( 0.0, 0.0, 40.0 )
                    upVector: Qt.vector3d( 0.0, 1.0, 0.0 )
                    viewCenter: Qt.vector3d( 0.0, 0.0, 0.0 )
                }

                //FirstPersonCameraController { camera: camera }
                CustomCameraController{
                    id: ctrl
                    camera: camera
                    linearSpeed: 50
                    windowSize: Qt.size(root.width, root.height)
                    rotationSpeed: 2.0
                }

               /* OrbitCameraController {
                    camera: camera
                    linearSpeed: 50
                    //lookSpeed: 100
                }*/

                components: [
                    RenderSettings {
                        activeFrameGraph: ForwardRenderer {
                            camera: camera
                            clearColor: "transparent"
                        }
                    },
                    InputSettings { }
                ]

               /*  TorusMesh {
                    id: torusMesh
                    radius: 5
                    minorRadius: 1
                    rings: 100
                    slices: 20
                }

                Transform {
                    id: torusTransform
                    scale3D: Qt.vector3d(1.5, 1, 0.5)
                    rotation: fromAxisAndAngle(Qt.vector3d(1, 0, 0), 45)
                }

                Entity {
                    id: torusEntity
                    components: [ torusMesh, material, torusTransform ]
                }*/

              /*  SphereMesh {
                    id: sphereMesh
                    radius: 3
                }*/

                PhongMaterial {
                     id: material
                     ambient: "red"
                }

                Timer{
                    id: tm
                    interval: 100
                    repeat: true
                    onTriggered: {
                        if (mesh.geometry){
                            if (scrt.fitView())
                                tm.stop()
                        }
                    }
                }

                Mesh{
                    id: mesh
                    onSourceChanged: {
                        tm.start()
                    }
                    onStatusChanged: function(aStatus){
                        if (aStatus !== Mesh.Ready)
                            tm.stop()
                    }
                }

                Transform {
                    id: transform
                    property real userAngle: 0.0
                    matrix: {
                        var m = Qt.matrix4x4();
                        //m.rotate(userAngle, Qt.vector3d(0, 1, 0))
                        //m.translate(Qt.vector3d(20, 0, 0));
                        return m;
                    }
                }

               /* NumberAnimation {
                    target: sphereTransform
                    property: "userAngle"
                    duration: 10000
                    from: 0
                    to: 360

                    loops: Animation.Infinite
                    running: true
                }*/

               /* GeometryRenderer {
                    id: mesh2
                    //几何体
                    //Geometry类型用于将Attribute对象列表分组在一起，
                    //以形成Qt3D能够使用GeometryRenderer渲染的几何形状。
                    instanceCount: 0
                    primitiveType: GeometryRenderer.Triangles
                    geometry: Geometry {
                        //属性Attribute，对应Shader中的attribute
                        Attribute {
                            attributeType: Attribute.VertexAttribute
                            vertexBaseType: Attribute.Float
                            vertexSize: 3
                            byteOffset: 0
                            byteStride: 3 * 4
                            count: 12
                            buffer: Buffer {
                                type: Buffer.VertexBuffer
                                data: new Float32Array(
                                          [
                                              -1.0,-1.0,-1.0,
                                                -1.0,-1.0, 1.0,
                                                -1.0, 1.0, 1.0,
                                                1.0, 1.0,-1.0,
                                                -1.0,-1.0,-1.0,
                                                -1.0, 1.0,-1.0,
                                                1.0,-1.0, 1.0,
                                                -1.0,-1.0,-1.0,
                                                1.0,-1.0,-1.0,
                                                1.0, 1.0,-1.0,
                                                1.0,-1.0,-1.0,
                                                -1.0,-1.0,-1.0,
                                                -1.0,-1.0,-1.0,
                                               -1.0, 1.0, 1.0,
                                                -1.0, 1.0,-1.0,
                                                1.0,-1.0, 1.0,
                                                -1.0,-1.0, 1.0,
                                                -1.0,-1.0,-1.0,
                                                -1.0, 1.0, 1.0,
                                                -1.0,-1.0, 1.0,
                                                1.0,-1.0, 1.0,
                                                1.0, 1.0, 1.0,
                                                1.0,-1.0,-1.0,
                                                1.0, 1.0,-1.0,
                                                1.0,-1.0,-1.0,
                                                1.0, 1.0, 1.0,
                                                1.0,-1.0, 1.0,
                                                1.0, 1.0, 1.0,
                                                1.0, 1.0,-1.0,
                                                -1.0, 1.0,-1.0,
                                                1.0, 1.0, 1.0,
                                                -1.0, 1.0,-1.0,
                                                -1.0, 1.0, 1.0,
                                                1.0, 1.0, 1.0,
                                                -1.0, 1.0, 1.0,
                                                1.0,-1.0, 1.0
                                          ])

                            }
                        }
                    }
                }*/

                Entity {
                    id: sphereEntity

                   /* GeometryRenderer{
                        id: myRenderer
                    }*/

                    components: [ mesh, material, transform]
                }
            }
        }
    }
    Component.onCompleted: {
        Pipelines().add(function(aInput){
            if (vw){
                vw.loadModel(aInput.data())
            }
        }, {name: "update3DAttr_" + name})
    }

    onVisibleChanged: {
        if (visible)
            vw = scene3d.createObject(root)
        else if (vw){
            vw.destroy()
            vw = null
        }
    }
    Keys.onPressed: {
        console.log("hi")
    }
}
