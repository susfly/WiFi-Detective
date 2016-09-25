#include <node.h>
#include <v8.h>
#include <uv.h>
#include <windows.h>
#include "Wlan.h"
#include "WlanInterface.h"
#include "Wlan_node.h"
#include <iostream>
using namespace std;
using namespace v8;

#define MAX_BSS_NUM 128

typedef struct wlan_event_cb_s {
    wlan_event_id_t id;
    uv_async_t async;
    Persistent<Function> event_cb;
} wlan_event_cb_t;

static wlan_event_cb_t g_event_cb[WLAN_EVENT_ID_MAX];

static CWlan wlan;
static INTFLIST *ifList;
static CWlanInterface *intf;

static void on_acm_notification_callback(WLANINTERFACE_NOTIFICATION *data, CWlanInterface *intf)
{
    uv_async_t* async;

    switch (data->code) {
        case wlan_notification_acm_disconnected:
            async = &g_event_cb[WLAN_ACM_DISCONNECTED].async;
            uv_async_send(async);
            break;
        case wlan_notification_acm_scan_complete:
            async = &g_event_cb[WLAN_ACM_SCAN_COMPLETE].async;
            uv_async_send(async);
            break;
        case wlan_notification_acm_connection_complete:
            async = &g_event_cb[WLAN_ACM_CONNECT_COMPLETE].async;
            uv_async_send(async);
            break;
        default:
            cout << "ignored acm notification code: " << data->code << endl;
        return;
    }
}

static void on_msm_notification_callback(WLANINTERFACE_NOTIFICATION *data, CWlanInterface *intf)
{
    switch (data->code) {
        case wlan_notification_msm_roaming_end:
            uv_async_send(&g_event_cb[WLAN_MSM_ROAMING_END].async);
            break;
        case wlan_notification_msm_adapter_removal:
            uv_async_send(&g_event_cb[WLAN_MSM_ADAPTER_REMOVAL].async);
            break;
        default:
            cout << "ignored msm notification code: " << data->code << endl;
        return;
    }
}

void notification_callback(WLANINTERFACE_NOTIFICATION *data, CWlanInterface *intf)
{
    cout << "guid: " << intf->GetGUIDStr() << endl;
    cout << "desc: " << intf->GetDescription() << endl;
    cout << "source: " << data->source << "\tcode: " << data->code << endl;
    cout << "noti_str: " << intf->FormatNotificationStr(data->source, data->code) << endl;

    switch (data->source) {
        case WLAN_NOTIFICATION_SOURCE_ACM:
            on_acm_notification_callback(data, intf);
            break;
        case WLAN_NOTIFICATION_SOURCE_MSM:
            on_msm_notification_callback(data, intf);
            break;
        default:
            return;
    }
}

bool WINAPI print(CWlanInterface *intf, LPVOID context)
{
    cout << intf->GetDescription() << endl;
    cout << intf->GetGUIDStr() << endl;

    return false;
}

void init(const FunctionCallbackInfo<Value>& args)
{
    Isolate* isolate = args.GetIsolate();

    wlan.WlanInit();
    ifList = wlan.GetInterfaces();
    cout << wlan.GetInterfacesNum() << endl;
    wlan.ForeachInterface(print, NULL);
    intf = wlan.GetFirstInterface();
    if (intf == NULL) {
        args.GetReturnValue().Set(false);
        printf("init_wlan fail, no interface\n");
        return;
    }
    intf->SetNotifyCallback(notification_callback);
    args.GetReturnValue().Set(true);
    printf("init_wlan success\n");
    return;
}

void dispose(const v8::FunctionCallbackInfo<v8::Value>&args)
{
    wlan.WlanDeInit();
}

void node_callback(uv_async_t* async)
{
    Isolate * isolate = Isolate::GetCurrent();
    printf("node_callback async %p, async->data %p", async, async->data);
    if (async->data == NULL) {
        return;
    }
    wlan_event_id_t id = *(wlan_event_id_t *)(async->data);
    cout << "node_callback " << id << endl;
    if (id < 0 || id > WLAN_EVENT_ID_MAX) {
        return;
    }
    cout << "node_callback wlan_event_id: " << id << endl;
    Local<Function>::New(isolate, g_event_cb[id].event_cb)->Call(isolate->GetCurrentContext()->Global(), 0, nullptr);
}

void add_event_listener(const v8::FunctionCallbackInfo<v8::Value>&args)
{
    Isolate* isolate = args.GetIsolate();
    if (!args[0]->IsNumber()) {
        args.GetReturnValue().Set(false);
        return;
    }
    int id = (int)args[0]->NumberValue();
    if (id < 0 || id > WLAN_EVENT_ID_MAX) {
        args.GetReturnValue().Set(false);
        return;
    }
    cout << "add_event_listener wlan_event_id: " << id << endl;
    Local<Function> callback = Local<Function>::Cast(args[1]);
    g_event_cb[id].id = (wlan_event_id_t)id;
    g_event_cb[id].event_cb.Reset(isolate, callback);
    g_event_cb[id].async.data = &(g_event_cb[id].id);
    printf("add_event_listener async %p, data %p\n", &(g_event_cb[id].async), g_event_cb[id].async.data);
    uv_async_init(uv_default_loop(), &(g_event_cb[id].async), node_callback);
    args.GetReturnValue().Set(true);
    return;
}

static inline Local<Object> wrap_bssinfo(Isolate* isolate, PBSS_INFO info)
{
    char buf[16];

    Local<String> ssid = String::NewFromUtf8(isolate, info->ssid);
    unsigned char *mac = (unsigned char *)info->bssid;
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%02x%02x.%02x%02x.%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    Local<String> bssid = String::NewFromUtf8(isolate, buf);
    Local<Number> rssi = Number::New(isolate, info->lRssi);
    Local<Integer> channel = Integer::New(isolate, info->ulChCenterFrequency);
    Local<Number> rxRate = Number::New(isolate, info->ulRxRate);
    Local<Number> txRate = Number::New(isolate, info->ulTxRate);

    Local<Object> obj = Object::New(isolate);
    obj->Set(String::NewFromUtf8(isolate, "ssid"), ssid);
    obj->Set(String::NewFromUtf8(isolate, "bssid"), bssid);
    obj->Set(String::NewFromUtf8(isolate, "rssi"), rssi);
    obj->Set(String::NewFromUtf8(isolate, "channel"), channel);
    obj->Set(String::NewFromUtf8(isolate, "rxRate"), rxRate);
    obj->Set(String::NewFromUtf8(isolate, "txRate"), txRate);

    return obj;
}

void get_cur_bssinfo(const v8::FunctionCallbackInfo<v8::Value>&args)
{
    Isolate* isolate = args.GetIsolate();
    if (intf == NULL) {
        args.GetReturnValue().Set(String::NewFromUtf8(isolate, "NotFound"));
        return;
    }

    if (!intf->IsConnect()) {
        args.GetReturnValue().Set(String::NewFromUtf8(isolate, "NotConnect"));
        return;
    }

    intf->UpdateCurBssInfo();
    PBSS_INFO info = intf->GetCurBssInfo();
    Local<Object> obj = wrap_bssinfo(isolate, info);
    args.GetReturnValue().Set(obj);
}

void get_intf_guid(const v8::FunctionCallbackInfo<v8::Value>&args)
{
    Isolate* isolate = args.GetIsolate();
    if (intf == NULL) {
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    Local<String> guid = String::NewFromUtf8(isolate, intf->GetGUIDStr());
    args.GetReturnValue().Set(guid);
    return;
}

void get_intf_info(const v8::FunctionCallbackInfo<v8::Value>&args)
{
    Isolate* isolate = args.GetIsolate();
    if (intf == NULL) {
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }

    Local<String> desc = String::NewFromUtf8(isolate, intf->GetDescription());
    Local<String> guid = String::NewFromUtf8(isolate, intf->GetGUIDStr());
    Local<Object> obj = Object::New(isolate);
    obj->Set(String::NewFromUtf8(isolate, "desc"), desc);
    obj->Set(String::NewFromUtf8(isolate, "guid"), guid);
    args.GetReturnValue().Set(obj);
}

void get_bss_list(const v8::FunctionCallbackInfo<v8::Value>&args)
{
    Isolate* isolate = args.GetIsolate();
    int cnt = MAX_BSS_NUM;
    int channel = 0, sameChanCnt = 0;
    int band = 0;

    if (intf == NULL) {
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }

    if (args[0]->IsNumber()) {
        channel = (int)args[0]->NumberValue();
    }

    PBSS_INFO pBss = (PBSS_INFO)malloc(sizeof(BSS_INFO) * cnt);
    if (pBss == NULL) {
        args.GetReturnValue().Set(Number::New(isolate, -1));
        return;
    }

    int ret = intf->GetBssList(NULL, pBss, &cnt);
    if (ret != ERROR_SUCCESS) {
        args.GetReturnValue().Set(Number::New(isolate, 0 - ret));
        return;
    }

    Local<Array> objs = Array::New(isolate, cnt);
    for (int i = 0; i < cnt; i++) {
        Local<Object> obj = wrap_bssinfo(isolate, &(pBss[i]));
        objs->Set(i, obj);
        if (channel != 0 && pBss[i].ulChCenterFrequency == channel) {
            sameChanCnt++;
        }
        if (pBss[i].ulChCenterFrequency > 15) {
            band = 1;
        }
    }
    Local<Object> retObj = Object::New(isolate);
    Local<Number> sameCnt = Number::New(isolate, sameChanCnt);
    Local<Number> bandSpt = Number::New(isolate, band);
    retObj->Set(String::NewFromUtf8(isolate, "sameChanCnt"), sameCnt);
    retObj->Set(String::NewFromUtf8(isolate, "band"), bandSpt);
    retObj->Set(String::NewFromUtf8(isolate, "bssArray"), objs);

    args.GetReturnValue().Set(retObj);
    return;
}

void init_module(Handle<Object> exports)
{
    NODE_SET_METHOD(exports, "dispose", dispose);
    NODE_SET_METHOD(exports, "init", init);
    NODE_SET_METHOD(exports, "addEventListener", add_event_listener);
    NODE_SET_METHOD(exports, "getCurBssinfo", get_cur_bssinfo);
    NODE_SET_METHOD(exports, "getIntfGuid", get_intf_guid);
    NODE_SET_METHOD(exports, "getIntfInfo", get_intf_info);
    NODE_SET_METHOD(exports, "getBssList", get_bss_list);
}

NODE_MODULE(wlan, init_module)
