'use strict';
// In renderer process (web page).
const {ipcRenderer} = require('electron');

const wlan = require("../build/Release/wlan");

const network = require("../build/Release/network");

const ping = require ("net-ping");

var wifidb = require("./js/sqlite3/wifi-sqlite3.js");

const notifier = require('node-notifier');

var closeEl = document.getElementById('close-window');

require('electron-cookies');

var isEventPieChartInit = false;

var mainGuid = null;

var band = 0;

var maxrate = 0;

var flownum = 0;

var score = 0;

var cnctCompTime;

var hasIP = false;

var hasAdapter = false;

var hasConnect = false;

var hasRecordLowIP = false;

var timelineCnt = 0;

var isDownloadUpdating = false;

var isCheckUpdating = false;

var welcomeStr = ['听说用WiFi侦探的人都很有眼光', '就等你了，来的正是时候', '别墨迹，快看看你的WiFi咋样了', '一直默默守护在原地的是我', 'WiFi好不好用，一看便知'];

Date.prototype.Format = function (fmt) { //author: meizz
    var o = {
        "M+": this.getMonth() + 1, //月份
        "d+": this.getDate(), //日
        "h+": this.getHours(), //小时
        "m+": this.getMinutes(), //分
        "s+": this.getSeconds(), //秒
        "q+": Math.floor((this.getMonth() + 3) / 3), //季度
        "S": this.getMilliseconds() //毫秒
    };
    if (/(y+)/.test(fmt)) fmt = fmt.replace(RegExp.$1, (this.getFullYear() + "").substr(4 - RegExp.$1.length));
    for (var k in o) {
        if (new RegExp("(" + k + ")").test(fmt)) fmt = fmt.replace(RegExp.$1, (RegExp.$1.length == 1) ? (o[k]) : (("00" + o[k]).substr(("" + o[k]).length)));
    }

    return fmt;
}

closeEl.addEventListener('click', function () {
    exit();
});

var session = ping.createSession ();
var delay;
var pingfunc = function(target) {
    if (target == "0.0.0.0") {
        delay = "-";
        return;
    }
    session.pingHost (target, function (error, target, sent, rcvd) {
    delay = rcvd - sent;
    if (error) {
        console.log (target + ": " + error.toString ());
        delay = "-";
    } else {
        //console.log (target + ": Alive (ms=" + delay + ")");
    }
});}

var getPingDelay = function() {
    return delay;
}

var scanCompleteCb = function(message) {
    console.time("scanCompleteCb");
    var now = new Date();
    var bssinfo = wlan.getCurBssinfo();
    var curCh = 0;
    if (bssinfo != undefined && bssinfo != "NotFound" && bssinfo != "NotConnect") {
        curCh = bssinfo.channel;
    }
    var bssListInfo = wlan.getBssList(curCh);
    $("#sameChanCnt").html(bssListInfo.sameChanCnt);
    if (band == 0 && bssListInfo.band == 1) {
        $("#band").html("支持5G");
        band = 1;
        wifidb.updateAdapterInfoBand(wlan.getIntfGuid(), band, function(err){
            updateAdapterScore(maxrate, band, flownum);
            if (err) {
                console.log(err)
            }
        });
    }

    console.log("I've been called! " + now);
    console.timeEnd("scanCompleteCb");
}

var netInfo;
var updateNetInfo = function () {
    if (hasConnect == false) {
        $("#ipStr").html("0.0.0.0");
        $("#ipGwStr").html("0.0.0.0");
        return;
    }
    netInfo = network.getNetworkInfo();
    /* no gateway ip */
    if (netInfo.ipGwStr == "0.0.0.0") {
        console.log("error gwip");
        /* if has connected, then no gwip means warning */
        if (hasConnect == true) {
            $("#ipStr").css("color", "#cc6666");
            $("#ipGwStr").css("color", "#cc6666");
            /* if has connect, and no ip, and time too long, add event to show slow */
            console.log("cnctCompTime " + cnctCompTime)
            if (cnctCompTime != null) {
                var now = new Date();
                var interval = now.getTime() - cnctCompTime.getTime();
                console.log(interval + 'ms');
                if (interval > 10000 && hasRecordLowIP == false) {
                    addEventTimeLine("7", "IP地址获取很慢，已耗时" + interval + 'ms');
                    hasRecordLowIP = true;
                }
            }
        } else {  /* if not connect or no adapter, not care */
            /* don't care */
        }
    } else {
        $("#ipStr").css("color", "#65cea7");
        $("#ipGwStr").css("color", "#65cea7");
        if (cnctCompTime != null) {
            var now = new Date();
            var interval = now.getTime() - cnctCompTime.getTime();
            console.log(interval + 'ms');
            addEventTimeLine("6", "IP地址获取成功，耗时" + interval + 'ms');
            cnctCompTime = null;
            hasRecordLowIP = false;
        }
        hasIP = true;
        hasRecordLowIP = false;
    }
    console.log(netInfo);
    $("#ipStr").html(netInfo.ipStr);
    $("#ipGwStr").html(netInfo.ipGwStr);
    $("#mac").html(netInfo.mac);
}

var pingGateWay = function () {
    if (netInfo != null && netInfo != undefined) {
        pingfunc(netInfo.ipGwStr);
    }
}

var TimerFunc;
var timeTicket;

var changeWifiRes = function (bssinfo) {
    if (bssinfo == "NotFound") {
        $("#signal-res-img").attr("src", "images/wifi/wifi-signal-bad.png");
        $("#signal-res-txt").html("");
        timelineBssInfo = "";
        $("#header_title").html("抱歉，看来你需要一张无线网卡");
    } else if (bssinfo == "NotConnect") {
        $("#signal-res-img").attr("src", "images/wifi/wifi-signal-bad.png");
        $("#signal-res-txt").html("");
        timelineBssInfo = "";
        $("#header_title").html("连接无线网络，开启更多精彩");
    } else if (bssinfo != undefined) {
        if (bssinfo.rssi >= -55) {
            $("#signal-res-img").attr("src", "images/wifi/wifi-signal-good.png");
            $("#signal-res-txt").html("好");
            $("#header_title").html("您的无线信号棒棒的，请继续保持住");
        } else if (bssinfo.rssi < -55 && bssinfo.rssi >= -65) {
            $("#signal-res-img").attr("src", "images/wifi/wifi-signal-normal.png");
            $("#signal-res-txt").html("中");
            $("#header_title").html("信号变弱了，网速可能会变慢哦");
        } else {
            $("#signal-res-img").attr("src", "images/wifi/wifi-signal-bad.png");
            $("#signal-res-txt").html("差");
            $("#header_title").html("信号好差，换个姿势靠近一点看看");
        }
        timelineBssInfo = bssinfo;
    } else {
        $("#signal-res-img").attr("src", "images/wifi/wifi-signal-bad.png");
        $("#signal-res-txt").html("");
        timelineBssInfo = "";
    }
}

var updateAdapterScore = function (maxrate, band, flownum) {
    if (band == 1) {
        if (score < 90) {
            score = 90;
        }
    } else if (maxrate > 72 || flownum > 1) {
        if (score < 80) {
            score = 80;
        }
    } else {
        if (score < 60) {
            score = 60;
        }
    }
    renderAdapterChart(score);
    wifidb.updateAdapterInfoScore(wlan.getIntfGuid(), score, function(err) {if (err) console.log(err)});
}

var renderAdapterChart = function (score) {
    var option_adapter_tmp  = adapterChart.getOption();
    option_adapter_tmp.series[0].data[1].value = score;
    option_adapter_tmp.series[0].data[1].name = ((score >= 90) ? "好" : (score >= 80 ? "中" : "差"));
    option_adapter_tmp.series[0].data[0].value = 100 - score;
    adapterChart.setOption(option_adapter_tmp);
}

var connectCompleteCb = function(isInit) {
    var bssinfo = wlan.getCurBssinfo();
    if (bssinfo == "NotFound") {
        $("#ssid").html("木有无线网卡");
        $("#channel").html("CH." + "N/A");
        $("#maxRate").html("N/A");
        $("#bssid").html("N/A");
        hasAdapter = false;
    } else if (bssinfo == "NotConnect") {
        $("#ssid").html("请先连接无线网");
        $("#channel").html("CH." + "N/A");
        $("#maxRate").html("N/A");
        $("#bssid").html("N/A");
        hasConnect = false;
    } else if (bssinfo != undefined) {
        console.log(bssinfo);
        $("#ssid").html(bssinfo.ssid);
        $("#channel").html("CH." + bssinfo.channel);
        if (bssinfo.txRate > maxrate) {
            maxrate = bssinfo.txRate;
            $("#maxRate").html(bssinfo.txRate / 1000 + " Mbps");
            wifidb.updateAdapterInfoMaxRate(wlan.getIntfGuid(), maxrate, function(err){
                updateAdapterScore(maxrate, band, flownum);
                if (err) {
                    console.log(err)
                }
            });
        } else {
            $("#maxRate").html(maxrate / 1000 + " Mbps");
        }

        if (flownum > 1) {
            $("#flownum").html("至少双流");
        } else {
            $("#flownum").html((bssinfo.txRate / 1000 <= 72 ? "单流" : "至少双流"));
            flownum = bssinfo.txRate / 1000 < 72 ? 1 : 2;
            wifidb.updateAdapterInfoFlownum(wlan.getIntfGuid(), flownum, function(err){
                updateAdapterScore(maxrate, band, flownum);
                if (err) {
                    console.log(err)
                }
            });
        }

        $("#bssid").html(bssinfo.bssid);
        if (band == 0 && bssinfo.channel > 15) {
            $("#band").html("支持5G");
            band = 1;
            wifidb.updateAdapterInfoBand(wlan.getIntfGuid(), band, function(err){
                updateAdapterScore(maxrate, band, flownum);
                if (err) {
                    console.log(err)
                }
            });
        }
        if (TimerFunc != undefined) {
            clearInterval(timeTicket);
            timeTicket = setInterval(TimerFunc, 1000);
        }
        hasConnect = true;
    } else {
        console.log(bssinfo);
    }
    changeWifiRes(bssinfo);
    if (isInit != true) {
        hasIP = false;
        hasRecordLowIP = false;
        console.log("connectCompleteCb " + hasIP);
        cnctCompTime = new Date();
        addEventTimeLine('1', "WiFi关联成功");
        notifier.notify({
            'title': 'WiFi侦探',
            'message': '当前无线网络已连接'
        });
        /* when connect complete, wait for 3s to update ip Address  */
        setTimeout(updateNetInfo(), 3000);
    } else {
        cnctCompTime = null;
        notifier.notify({
            'title': 'WiFi侦探',
            'message': welcomeStr[Math.floor(Math.random()*5)]
        });
        updateNetInfo();
    }
}

var discCallback = function() {
    var bssinfo = wlan.getCurBssinfo();
    console.log("discCallBack" + bssinfo);
    if (bssinfo == "NotConnect") {
        $("#ssid").html("请先连接无线网～");
        $("#channel").html("CH." + "N/A");
        $("#maxRate").html("N/A");
        $("#bssid").html("N/A");
        hasConnect = false;
    }
    changeWifiRes(bssinfo);
    addEventTimeLine('2', "WiFi断开连接");
    /* when disconnect, clear netInfo */
    netInfo = null;
    if (timeTicket != undefined) {
        clearInterval(timeTicket);
    }
    hasIP = false;
    hasRecordLowIP = false;
    // Object
    notifier.notify({
        'title': 'WiFi侦探',
        'message': '无线网络已断开'
    });
    updateNetInfo();
}

var roamEndCallback = function() {
    addEventTimeLine('4', "完成一次漫游");
    var bssinfo = wlan.getCurBssinfo();
    changeWifiRes(bssinfo);
    notifier.notify({
        'title': 'WiFi侦探',
        'message': '刚刚您的无线网卡发生一次漫游'
    });
    hasIP = false;
    hasRecordLowIP = false;
    /* when roam end, wait for 3s to update ip Address  */
    setTimeout(updateNetInfo(), 3000);
}

var adapterRemovalCallback = function() {
    addEventTimeLine('5', "网卡被移除");
}

var renderTimeLine = function (type, timeStr, message) {

    if (timeStr == null || timeStr == undefined) {
        return;
    }
    var isnormal;
    if (type == '7') {
        isnormal = false;
    } else {
        isnormal = true;
    }

    $(".timeline").prepend(
        '<article>'
            + '<div class="timeline-desk">'
              + '<div class="panel">'
                   + '<div class="panel-body">'
                       + '<span class="arrow"></span>'
                            + '<span class="timeline-icon"></span>'
                            + '<span class="timeline-date"></span>'
                            + '<h1 class="green" style="color: #a0a0a0;">' + timeStr + '</h1>'
                       + (isnormal ? ('<p style="color: #65cea7;">' + message + '</p>') : ('<p style="color: #cc6666;">' + message + '</p>'))
                   + '</div>'
               + '</div>'
           + '</div>'
        + '</article>'
    );
    timelineCnt++;
    if (timelineCnt > 3) {
        $(".major-record").css("background","");
    }
}

var timelineBssInfo;
var addEventTimeLine = function (type, message) {
    var timeStr = new Date().Format("yyyy-MM-dd hh:mm:ss");
    renderTimeLine(type, timeStr, message);
    wifidb.insertWifiEvent(timeStr, type, function(err){
        if (err) {
            console.log(err);
            return;
        }
        if (refreshEventPie != undefined) {
            refreshEventPie();
        }
    });
}

var exit = function (type, message) {
    var timeStr = new Date().Format("yyyy-MM-dd hh:mm:ss");
    renderTimeLine(type, timeStr, "小侦探休息咯");
    wifidb.insertWifiEvent(timeStr, '3', function(err){
        ipcRenderer.send('close-main-window');
        if (err) {
            console.log(err);
            return;
        }
    });
}

var sqlInitFuncCB = function(error){
    if (error) {
        console.log(error);
        return;
    }
    console.log("init db success");
};

var timelineDataInit = function () {
    /* get last 7 days data */
    var d = new Date();
    d.setDate(d.getDate() - 7);
    var ts_from = d.Format("yyyy-MM-dd hh:mm:ss");
    var ts_to = new Date().Format("yyyy-MM-dd hh:mm:ss");
    console.log("ts_from " + ts_from + " ts_to " + ts_to);
    wifidb.forAllWifiEventByTs(ts_from, ts_to, function(err, row){
        if (err) {
            console.log(err);
            return;
        }
        renderTimeLine(row.type, row.ts, row.message);
    }, function (err, nothing) {
        addEventTimeLine('0', "小侦探来咯");
        if (err) {
            console.log(err);
            return;
        }
    });
}

var initEventType = function () {
    wifidb.insertEventType('3', '小侦探休息咯', function(err){if(err)console.log(err)});
    wifidb.insertEventType('2', 'WiFi断开连接', function(err){if(err)console.log(err)});
    wifidb.insertEventType('1', 'WiFi关联成功', function(err){if(err)console.log(err)});
    wifidb.insertEventType('0', '小侦探来咯', function(err){if(err)console.log(err)});
    wifidb.insertEventType('4', '完成一次漫游', function(err){if(err)console.log(err)});
    wifidb.insertEventType('5', '网卡被移除', function(err){if(err)console.log(err)});
    wifidb.insertEventType('6', 'IP地址获取成功', function(err){if(err)console.log(err)});
    wifidb.insertEventType('7', 'IP地址获取很慢', function(err){if(err)console.log(err)});
}

var sqlInit = function () {
    wifidb.connect(sqlInitFuncCB);
    wifidb.setupWifiEvent(sqlInitFuncCB);
    wifidb.setupEventType(function(err){
        if (err) {
            console.log(err);
            return;
        }
        wifidb.forAllEventType(function (err, data) {
            if (err) {
                console.log(err);
                return;
            }
            console.log(data);
            if (data[0].cnt == 0) {
                initEventType();
            }
        });

    });
    wifidb.setupAdapterInfo(sqlInitFuncCB);
};

var initAdapterInfo = function(err, data) {
    var intf_info = wlan.getIntfInfo();
    if (intf_info == null) {
        console.log("no intf info");
        return;
    }
    hasAdapter = true;
    $("#desc").html(intf_info.desc);
    $("#band").html("2.4G");
    wifidb.findWifiAdapterByGuid(intf_info.guid, function (err, data) {
        if (err) {
            console.log(err);
            connectCompleteCb(true);
            return;
        }
        if (data.cnt == 0) {
            wifidb.insertAdapterInfo(intf_info.guid, intf_info.desc, function(err){
                if (err) {
                    console.log(err);
                }
                connectCompleteCb(true);
            });
        } else {
            band = data.band;
            maxrate = data.maxrate;
            flownum = data.flownum;
            score = data.score;
            connectCompleteCb(true);
            renderAdapterChart(score);
            if (band == 1) {
                $("#band").html("2.4G/5G");
            } else {
                $("#band").html("2.4G");
            }
            if (flownum > 1) {
                $("#flownum").html("至少双流");
            }
        }

    });
}

var init = function() {
    /* init entity */
    wlan.init();
    var guid = wlan.getIntfGuid();
    if (guid == null || guid == undefined) {
        sqlInit();
        return;
    }
    guid = guid.toUpperCase();
    mainGuid = guid;
    console.log(guid);
    var ret = network.init(guid);
    console.log("network init " + ret);
    sqlInit();
    /* first update current bss info */
    //connectCompleteCb(true);
    initAdapterInfo();
    /* first init wlan interface info */
    /* add callback */
    /* scan complete event 0 */
    wlan.addEventListener(0, scanCompleteCb);
    /* connect complete event 1 */
    wlan.addEventListener(1, connectCompleteCb);
    /* disconnect complete event 2 */
    wlan.addEventListener(2, discCallback);
    /* roam end event 3 */
    wlan.addEventListener(3, roamEndCallback);
    /* adapter removal event 4 */
    wlan.addEventListener(4, adapterRemovalCallback);

    timelineDataInit();
    //dataInit();
};

init();

var http = require('http');
var fs = require('fs');

var download = function(url, dest, cb) {
  var file = fs.createWriteStream(dest);
  var request = http.get(url, function(response) {
    response.pipe(file);
    file.on('finish', function() {
      file.close(cb);  // close() is async, call cb after close completes.
    });
  }).on('error', function(err) { // Handle errors
    fs.unlink(dest); // Delete the file async. (But we don't check the result)
    if (cb) cb(err.message);
  });
};

$("#checkUpdateBtn").click(function(){
    if (isCheckUpdating == true) {
        return;
    }
    if (isDownloadUpdating == true) {
        return;
    }
    //console.time("checkUpdateBtn")
    //download("http://wis.ruijie.com.cn/assets/wis-run/update.zip", "./update.zip", function(){console.log("download finish")});
    //download("http://192.168.202.159/app.zip", "./apptmp.zip", function(){console.log("download finish")});
    //console.timeEnd("checkUpdateBtn")
});
