//require("./grid/style.css")
require("./grid/style.scss")
let rea = require('reajs')

window.onload = function(aInput){
    rea.pipelines().run("jsWindowLoaded", "")
}

import GridLayout from 'react-grid-layout'
import "regenerator-runtime/runtime"
import React from "react"
import ReactDOM from "react-dom"
import "react-grid-layout/css/styles.css"
import "react-resizable/css/styles.css"

const layout0 = [
  //{i: 'a', x: 0, y: 0, w: 1, h: 2},
  //{i: 'b', x: 1, y: 0, w: 3, h: 2},
  //{i: 'c', x: 4, y: 0, w: 1, h: 2}
];
const gridheight = 30
let whole_width = document.body.clientWidth
class MyFirstGrid extends React.Component {
  constructor(props) {
    super(props)
    this.state = {layout: layout0, scrollTop: 0}
    window.onresize = e => {
      whole_width = document.body.clientWidth
      this.setState({})
    }
    rea.pipelines().add(aInput => {
      this.layout_modification = aInput.scope()
      this.setState({})
      this.setState({layout: aInput.data()})
      //aInput.outs(this.state.layout, "layoutChanged") //will trig onLayoutChange
    }, {name: "updateLayout"})
  }
  componentDidMount() {
    window.addEventListener("scroll", e => {
      this.state.scrollTop = e.srcElement.scrollTop
      this.handleLayoutChange(this.state.layout)
    }, true)
  }
  handleLayoutChange = aLayout=>{
    let ret = []
    for (let i in aLayout)
      ret.push({i: aLayout[i].i, x: aLayout[i].x, y: aLayout[i].y, w: aLayout[i].w, h: aLayout[i].h, dely: this.state.scrollTop})
    this.state.layout = aLayout
    rea.pipelines().run("layoutChanged", ret, "", this.layout_modification)
  }
  render() {
    let cls = "layout layout-scroll"
    return (
      <GridLayout className={cls} layout={this.state.layout} autoSize={false} margin={[0, 0]} onLayoutChange={this.handleLayoutChange} rowHeight={gridheight} width={whole_width}>
        {
          this.state.layout.map((item, index)=>{
            return <div key={item.i}></div>
          })
        }
      </GridLayout>
    )
  }
}

window.onload = e => {
  rea.pipelines("qml").init(function(){
    rea.pipelines().run("reaGridLoaded", "")
  })
}

const element = (
    <MyFirstGrid />
)

ReactDOM.render(
    element,
    document.body
)