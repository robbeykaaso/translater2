global.rea = require('reajs')
require("./chart/style.scss")

const cht = require("echarts")

const chart = cht.init(document.getElementById("chart"))

var option = {
    title: {
        text: 'ECharts entry example'
    },
    tooltip: {},
    legend: {
        data:['Sales']
    },
    xAxis: {
        data: ["shirt","cardign","chiffon shirt","pants","heels","socks"]
    },
    yAxis: {},
    series: [{
        name: 'Sales',
        type: 'bar',
        data: [5, 20, 36, 10, 10, 20]
    }]
};

//chart.setOption(option);

window.addEventListener("resize", function(e){
    chart.resize()
})

rea.pipelines("c++").init(function(){
    rea.pipelines().add(function(aInput){
        chart.clear()
        chart.setOption(aInput.data())
        aInput.out()
    }, {name: "updateEChartAttr_" + rea.env().tag})
})