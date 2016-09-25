// 基于准备好的dom，初始化echarts实例
var signalChart = echarts.init(document.getElementById('signalchart'));
var cur_key = "rssi";

var chartObjmap = {};

function CreatChartObj(key, func, el, name, subtext) {
    var obj = new Object();
    obj.data = [];
    obj.max = 0;
    obj.min = 0;
    obj.time = [];
    obj.func = func;
    obj.el = el;
    obj.key = key;
    obj.name = name;
    obj.subtext = subtext;
    chartObjmap[key] = obj;

    return obj;
}

var speedObj = new Object();
var lastDnBytes = -1;
var lastUpBytes = -1;
speedObj.dnSpeed = 0;
speedObj.upSpeed = 0;
speedObj.rateMbps = 0;
var getNetState = function () {
    var netState = network.getNetworkState();
    speedObj.rateMbps = netState.rateMbps;
    if (lastDnBytes == -1) {
        lastDnBytes = netState.downBytes;
        lastUpBytes = netState.upBytes;
        return speedObj;
    }
    speedObj.dnSpeed = (((netState.downBytes - lastDnBytes) * 1.0) / 1024.0).toFixed(2);
    speedObj.upSpeed = (((netState.upBytes - lastUpBytes) * 1.0) / 1024.0).toFixed(2);
    lastDnBytes = netState.downBytes;
    lastUpBytes = netState.upBytes;
    return speedObj;
}

var getSpeedObj = function () {
    return speedObj;
}

var getBssInfoInTimer = function () {
    var bssinfo = wlan.getCurBssinfo();
    changeWifiRes(bssinfo);
    return bssinfo;
}

var rssiChartObj = CreatChartObj("rssi", getBssInfoInTimer, $("#rssi"), "信号强度", "RSSI(dbm)");
var linkrateChartObj = CreatChartObj("linkrate", getNetState, $("#linkrate"), "链路速率", "(Mbps)");
var dnSpeedChartObj = CreatChartObj("dnSpeed", getSpeedObj, $("#dnSpeed"), "下行速度", "(kbps)");
var upSpeedChartObj = CreatChartObj("upSpeed", getSpeedObj, $("#upSpeed"), "上行速度", "(kbps)");
var delayChartObj = CreatChartObj("delay", getPingDelay, $("#delay"), "空口时延", "ms");

option = {
    title : {
        text: '信号强度',
        subtext: 'RSSI(dbm)'
    },
    tooltip : {
        trigger: 'axis'
    },
    legend: {
        data:['信号强度']
    },
    toolbox: {
        show : false,
        feature : {
            mark : {show: true},
            dataView : {show: true, readOnly: false},
            magicType : {show: true, type: ['line', 'bar']},
            restore : {show: true},
            saveAsImage : {show: true}
        }
    },
    dataZoom: {
        show: false,
        start : 2
    },
    calculable : true,
    xAxis : [
        {
            type : 'category',
            boundaryGap : false,
            data : []
        }
    ],
    yAxis : [
        {
            type : 'value',
            axisLabel : {
                formatter: '{value}'
            },
            max:-30
        }
    ],
    series : [
        {
            name:'信号强度',
            type:'line',
            data: [],
            smooth:true,
            markPoint : {
                data : [
                    {type : 'max', name: '最大值'},
                    {type : 'min', name: '最小值'}
                ]
            },
            markLine : {
                data : [
                    {type : 'average', name: '平均值'}
                ]
            },
            itemStyle: {
                normal: {
                    color: '#2EC7C9'
                }
            }
        }
    ]
};

// 使用刚指定的配置项和数据显示图表。
signalChart.setOption(option);

var executeObjFunc = function (obj) {
    var now = new Date();
    var timeStr = [now.getHours(), now.getMinutes(), now.getSeconds()].join(":");

    var info = obj.func();
    if (info != undefined) {
        if (obj.data.length >= data_length) {
            obj.data.shift();
            obj.time.shift();
        }
        var value;
        switch (obj.key) {
            case "rssi":
                value = info.rssi;
                break;
            case "linkrate":
                value = info.rateMbps;
                break;
            case "dnSpeed":
                value = info.dnSpeed;
                break;
            case "upSpeed":
                value = info.upSpeed;
                break;
            case "delay":
                value = info;
                break;
            default:
                break;
        }
        if (value != undefined) {
            obj.el.html(value);
            obj.data.push(value);
            obj.time.push(timeStr);
            obj.max = Math.max.apply(null, obj.data) + 5;
            obj.min = Math.min.apply(null, obj.data) - 5;
        }
    }
}

var data_length = 20;

var refreshChart = function(key) {
    for (item in chartObjmap) {
        executeObjFunc(chartObjmap[item]);
    }
    var obj = chartObjmap[key];
    signalChart.setOption({
        title : {
            text: obj.name,
            subtext: obj.subtext
        },
        legend: {
            data:[obj.name]
        },
        xAxis : [{
                data : obj.time
            }],
        yAxis : [
            {
                max:obj.max,
                min:obj.min
            }
        ],
        series: [{
            name: obj.name,
            data: obj.data
        }]
    });
}

TimerFunc = function() {
    pingGateWay();
    refreshChart(cur_key);
    if (hasIP == false) {
        updateNetInfo();
    }
    //console.log("timer is call");
}

void function () {
    var bssinfo = wlan.getCurBssinfo();
    if (bssinfo == "NotConnect" || bssinfo == "NotFound") {
        console.log(bssinfo);
    } else {
        timeTicket = setInterval(TimerFunc, 1000);
    }
}();

var active_tab = "a_rssi";

$("#a_rssi").click(function() {
    if (mainGuid == null) {
        return;
    }
    if (cur_key == "rssi") {
        /* do nothing */
    } else {
        /* clear interval */
        cur_key = "rssi";
        refreshChart(cur_key);
    }
});

$("#a_linkrate").click(function() {
    if (mainGuid == null) {
        return;
    }
    if (cur_key == "linkrate") {
        /* do nothing */
    } else {
        cur_key = "linkrate";
        refreshChart(cur_key);
    }
});

$("#a_delay").click(function() {
    if (mainGuid == null) {
        return;
    }
    if (cur_key == "delay") {
        /* do nothing */
    } else {
        cur_key = "delay";
        refreshChart(cur_key);
    }
});
$("#a_dnSpeed").click(function() {
    if (mainGuid == null) {
        return;
    }
    if (cur_key == "dnSpeed") {
        /* do nothing */
    } else {
        cur_key = "dnSpeed";
        refreshChart(cur_key);
    }
});
$("#a_upSpeed").click(function() {
    if (mainGuid == null) {
        return;
    }
    if (cur_key == "upSpeed") {
        /* do nothing */
    } else {
        cur_key = "upSpeed";
        refreshChart(cur_key);
    }
});
