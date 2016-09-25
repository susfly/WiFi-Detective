//数据库接口库
var util = require('util');
var sqlite3 = require('sqlite3');

sqlite3.verbose();
var db = undefined;

/*
 数据库名是直接硬编码的，所以当调用connect和setup函数时，当前目录中就会生成chap06.sqlite3文件
 */

exports.connect = function(callback){
    console.log(__dirname + '/../../../../wifi.db');
    db = new sqlite3.Database(__dirname + '/../../../../wifi.db', sqlite3.OPEN_READWRITE | sqlite3.OPEN_CREATE,
        function(err){
            if (err){
                util.log('FAIL on creating database ' + err);
                callback(err);
            } else {
                callback(null);
            }
        });
}

//此处的disconnect函数是空的
exports.disconnect = function(callback){
    callback(null);
}

exports.setupWifiEvent = function(callback){
    db.run("CREATE TABLE IF NOT EXISTS T_WIFI_EVENT " +
        "(id INTEGER PRIMARY KEY AUTOINCREMENT, ts DATETIME, type VARCHAR(10), isUploaded VARCHAR(1))",
        function(err){
            if (err){
                util.log('FAIL on creating table ' + err);
                callback(err);
            } else {
                callback(null);
            }
        });
}

exports.setupEventType = function(callback) {
    db.run("CREATE TABLE IF NOT EXISTS T_WIFI_EVENT_TYPE " +
        "(type VARCHAR(10) PRIMARY KEY, message VARCHAR(255))",
        function(err){
            if (err) {
                util.log('FAIL ON CREATE TABLE T_WIFI_EVENT_TYPE ' + err);
                callback(err);
            } else {
                callback(null);
            }
        });
}

exports.setupAdapterInfo = function(callback) {
    db.run("CREATE TABLE IF NOT EXISTS T_WIFI_ADAPTER_INFO " +
        "(guid VARCHAR(64) PRIMARY KEY, mac varchar(32), name VARCHAR(255), band VARCHAR(10), maxrate VARCHAR(10), flownum VARCHAR(10), score VARCHAR(10))",
        function(err){
            if (err) {
                util.log('FAIL ON CREATE TABLE T_WIFI_ADAPTER_INFO ' + err);
                callback(err);
            } else {
                callback(null);
            }
        });
}

exports.wifiEvent = {"ts": "", "type": ""};
exports.insertWifiEvent = function(ts, type, callback){
    db.run("INSERT INTO T_WIFI_EVENT (ts, type, isUploaded) " +
        "VALUES (?, ?, ?);",
        [ts, type, '0'],
        function(error){
            if (error){
                util.log('FAIL on insertWifiEvent ' + error);
                callback(error);
            } else {
                callback(null);
            }
        });
}

exports.insertEventType = function(type, message, callback) {
    db.run("INSERT INTO T_WIFI_EVENT_TYPE (type, message) " +
        "VALUES (?, ?);",
        [type, message],
        function(error){
            if (error){
                util.log('FAIL on insertEventType ' + error);
                callback(error);
            } else {
                callback(null);
            }
        });
}

exports.insertAdapterInfo = function(guid, name, callback) {
    db.run("INSERT INTO T_WIFI_ADAPTER_INFO (guid, name) " +
        "VALUES (?, ?);",
        [guid, name],
        function(error){
            if (error){
                util.log('FAIL on insertEventType ' + error);
                callback(error);
            } else {
                callback(null);
            }
        });
}
/*
run函数接受一个字符串参数，其中?表示占位符，占位符的值必须通过一个数组传递进来
调用者提供了一个回调函数，然后通过这个回调函数来声明错误
 */

exports.deleteWifiEvent = function(ts, callback){
    db.run("DELETE FROM T_WIFI_EVENT WHERE ts <= ?;",
        [ts],
        function(err){
            if (err){
                util.log('FAIL to delete WifiEvent ' + err);
                callback(err);
            } else {
                callback(null);
            }
        });
}

exports.deleteEventType = function (type, callback) {
    db.run("DELETE FROM T_WIFI_EVENT_TYPE WHERE type = ?;",
        [type],
        function(err){
            if (err){
                util.log('FAIL to delete EventType ' + err);
                callback(err);
            } else {
                callback(null);
            }
        })
}

exports.updateWifiEvent = function(ts, type, callback){
    db.run("UPDATE T_WIFI_EVENT " +
        "SET ts = ?, type = ? " +
        "WHERE ts = ?",
        [ts, type, ts],
        function(err){
            if (err){
                util.log('FAIL on updating WifiEvent ' + err);
                callback(err);
            } else {
                callback(null);
            }
        });
}

exports.updateEventType = function(type, message, callback){
    db.run("UPDATE T_WIFI_EVENT_TYPE " +
        "SET type = ?, message = ? " +
        "WHERE type = ?",
        [type, message, type],
        function(err){
            if (err){
                util.log('FAIL on updating EventType ' + err);
                callback(err);
            } else {
                callback(null);
            }
        });
}

exports.updateAdapterInfoMac = function (guid, mac, callback) {
    db.run("UPDATE T_WIFI_ADAPTER_INFO SET mac = ? ",
        [mac],
        function(err) {
            if (err){
                util.log('FAIL on updating updateAdapterInfo mac ' + err);
                callback(err);
            } else {
                callback(null);
            }
        });
}

exports.updateAdapterInfoBand = function (guid, band, callback) {
    db.run("UPDATE T_WIFI_ADAPTER_INFO SET band = ? ",
        [band],
        function(err) {
            if (err){
                util.log('FAIL on updating updateAdapterInfo band ' + err);
                callback(err);
            } else {
                callback(null);
            }
        });
}

exports.updateAdapterInfoMaxRate = function (guid, maxRate, callback) {
    db.run("UPDATE T_WIFI_ADAPTER_INFO SET maxRate = ? ",
        [maxRate],
        function(err) {
            if (err){
                util.log('FAIL on updating updateAdapterInfo maxRate ' + err);
                callback(err);
            } else {
                callback(null);
            }
        });
}

exports.updateAdapterInfoFlownum = function (guid, flownum, callback) {
    db.run("UPDATE T_WIFI_ADAPTER_INFO SET flownum = ? ",
        [flownum],
        function(err) {
            if (err){
                util.log('FAIL on updating updateAdapterInfo flownum ' + err);
                callback(err);
            } else {
                callback(null);
            }
        });
}

exports.updateAdapterInfoScore = function (guid, score, callback) {
    db.run("UPDATE T_WIFI_ADAPTER_INFO SET score = ? ",
        [score],
        function(err) {
            if (err){
                util.log('FAIL on updating updateAdapterInfo score ' + err);
                callback(err);
            } else {
                callback(null);
            }
        });
}


exports.findWifiAdapterByGuid = function(guid, callback) {
    var didOne = false;
    console.log("findWifiAdapterByGuid @@@@ " + guid);
    db.each("SELECT count(*) as cnt, a.* FROM T_WIFI_ADAPTER_INFO a WHERE guid = ?",
        [guid],
        function(err, row){
            console.log("111111 " + err);
            if (err){
                util.log('FAIL to findWifiEventByTs row ' + err);
                callback(err, null);
            } else {
                if (!didOne){
                    callback(null, row);
                    didOne = true;   //保证回调函数只被执行一次
                }
            }
        });
}

exports.allWifiEvent = function(callback){
    util.log(' in WifiEvent');
    db.all("SELECT * FROM T_WIFI_EVENT", callback);
}

exports.forAllWifiEvent = function(doEach, done){
    db.each("SELECT * FROM T_WIFI_EVENT", function(err, row){
        if (err){
            util.log('FAIL to retrieve row ' + err);
            done(err, null);
        } else {
            doEach(null, row);
        }
    }, done);
}

exports.forAllWifiEventByTs = function(ts_from, ts_to, doEach, done){
    db.each("SELECT a.type,a.ts,b.message FROM T_WIFI_EVENT a, T_WIFI_EVENT_TYPE b " +
        "WHERE a.ts BETWEEN ? AND ? and a.type = b.type;",
        [ts_from, ts_to],
        function(err, row){
            if (err){
                util.log('FAIL to forAllWifiEventByTs row ' + err);
                done(err, null);
            } else {
                doEach(null, row);
            }
        }, done);
}

exports.forAllWifiEventRecentlyWeek = function(ts_from, ts_to, doEach, done){
    db.each("SELECT a.ts,b.message FROM T_WIFI_EVENT a, T_WIFI_EVENT_TYPE b " +
        "WHERE a.ts BETWEEN ? AND ? and a.type = b.type and julianday('now') - julianday(a.ts) <= 7;",
        [ts_from, ts_to],
        function(err, row){
            if (err){
                util.log('FAIL to forAllWifiEventByTs row ' + err);
                done(err, null);
            } else {
                doEach(null, row);
            }
        }, done);
}
/*
allNotes和forAll函数是操作所有数据的两种方法，allNotes把数据库中所有的数据行收集到一个数组里，
而forAll方法可以接受两个回调函数，每当从数据集中拿一行数据，回调函数doEach都会执行一遍，当读完所有数据时，回调函数done就会执行
 */

exports.findWifiEventByTs = function(ts_from, ts_to, callback) {
    var didOne = false;
    db.each("SELECT * FROM T_WIFI_EVENT WHERE ts BETWEEN ? AND ?",
        [ts_from, ts_to],
        function(err, row){
            if (err){
                util.log('FAIL to findWifiEventByTs row ' + err);
                callback(err, null);
            } else {
                if (!didOne){
                    callback(null, row);
                    didOne = true;   //保证回调函数只被执行一次
                }
            }
        });
}


exports.forAllEventStatWeek = function(callback){
    db.all("select count(*) as value, b.message as name, a.type from t_wifi_event a, t_wifi_event_type b " +
    "where a.type = b.type and a.type != 0 and a.type != 3 and a.type != 6 and julianday('now') - julianday(a.ts) <= 7 " +
    "group by a.type", callback);
}

exports.forAllEventType = function(callback){
    db.all("select count(*) as cnt from T_WIFI_EVENT_TYPE", callback);
}
