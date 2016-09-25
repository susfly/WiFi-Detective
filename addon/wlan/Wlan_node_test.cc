#include <node.h>
#include <v8.h>
#include <uv.h>
#include <windows.h>
#include "Wlan.h"
#include "WlanInterface.h"
#include <iostream>
using namespace std;
using namespace v8;

struct Work {
    uv_work_t    request;
    Persistent<Function> g_cb;
    Isolate * isolate;
    Local< Context > context;
};
Work *work;
CWlan wlan;
INTFLIST *ifList;
CWlanInterface *intf;
int g_event_id = 0;
uv_loop_t *loop;
uv_idle_t idler;
static uv_async_t s_async = {0};

void CallThisWithThis(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Function> cb = Local<Function>::Cast(args[0]);
    Local<Value> argv[1] = {args[1]};
    cb->Call(Null(isolate), 1, argv);
}

void Callback(WLANINTERFACE_NOTIFICATION *data, CWlanInterface *intf)
{
    //Isolate * isolate = (Isolate *)intf->GetUserParam();
    //HandleScope scope(isolate);
    //Isolate* isolate = work->isolate;
    //isolate->Exit();
    //v8::Unlocker unlocker(isolate);
    //std::cerr << "Worker thread trying to enter isolate" << std::endl;
    //v8::Locker locker(isolate);
    //isolate->Enter();

    //std::cerr << "Worker thread has entered isolate" << std::endl;
    // we need this to create local handles, since this
    // function is NOT called by Node.js
    //v8::HandleScope handleScope(isolate);
    printf("Callback isolate %p\n", work->isolate);
	cout << intf->GetGUIDStr() << "     " << intf->GetDescription() << "callback" << data->source << data->code << endl;
	cout << intf->FormatNotificationStr(data->source, data->code) << endl;
    g_event_id = 1;
    uv_async_send(&s_async);
    //Local<Function>::New(isolate, work->g_cb)->Call(isolate->GetCurrentContext()->Global(), 0, nullptr);
}

bool WINAPI print(CWlanInterface *intf, LPVOID context)
{
	cout << intf->GetDescription() << endl;
	cout << intf->GetGUIDStr() << endl;

	return false;
}

void testWlan(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	v8::String::Utf8Value str(args[0]->ToString());
    /* test func call */
	char *name = *str;
    printf("name: %s\n", name);

    /* test wlan call */


    wlan.WlanInit();
    ifList = wlan.GetInterfaces();
	cout << wlan.GetInterfacesNum() << endl;
	wlan.ForeachInterface(print, NULL);

    ifList = wlan.RefreshInterfaces();
	cout << wlan.GetInterfacesNum() << endl;
	wlan.ForeachInterface(print, NULL);
	const char *aaa="{833DFD79-E162-424b-9E86-AE87391F5033}";
	intf = wlan.FindInterfaceByGUID(aaa);
    if (intf == NULL) {
    args.GetReturnValue().Set(false);
    return;
    }

    intf->SetNotifyCallback(Callback);
    work->isolate = isolate;
    work->context = isolate->GetCurrentContext();
    //intf->SetUserParam(isolate);
    printf("testWlan isolate %p\n", isolate);
    printf("1111: %s\n", name);

    Local<Function>::New(isolate, work->g_cb)->Call(isolate->GetCurrentContext()->Global(), 0, nullptr);
    Local<String> retval = String::NewFromUtf8(isolate, intf->GetDescription());
    printf("2222: %s\n", name);
	args.GetReturnValue().Set(retval);
	return;
}

void RegistCallBackAsync(const v8::FunctionCallbackInfo<v8::Value>&args)
{
    printf("RegistCallBackAsync is call...\n");
    Isolate* isolate = args.GetIsolate();
    printf("RegistCallBackAsync isolate %p\n", isolate);
    Local<Function> callback = Local<Function>::Cast(args[0]);
    work = new Work();
    work->g_cb.Reset(isolate, callback);
    callback->Call(Null(isolate), 0, nullptr);
    args.GetReturnValue().Set(true);
    printf("RegistCallBackAsync is call end...\n");
}

void deInit(const v8::FunctionCallbackInfo<v8::Value>&args)
{
    work->g_cb.Reset();
    delete work;
}

static void WorkAsync(uv_work_t *req)
{
    //Isolate * isolate = Isolate::GetCurrent();
    //printf("WorkAsync isolate %p\n", isolate);
    Work *work = static_cast<Work *>(req->data);
    printf("WorkAsync do nothing...\n");\
    while(1) {
        if (g_event_id == 1) {
            g_event_id = 0;
            printf("WorkAsync get g_event_id...\n");
            break;
        }
        Sleep(2000);
    }
}

// called by libuv in event loop when async function completes
static void WorkAsyncComplete(uv_work_t *req,int status)
{
    Isolate * isolate = Isolate::GetCurrent();
    printf("WorkAsyncComplete isolate %p\n", isolate);
    // Fix for Node 4.x - thanks to https://github.com/nwjs/blink/commit/ecda32d117aca108c44f38c8eb2cb2d0810dfdeb
    v8::HandleScope handleScope(isolate);

    Local<Array> result_list = Array::New(isolate);
    Work *work = static_cast<Work *>(req->data);
    printf("WorkAsyncComplete do nothing...\n");
    // execute the callback
    // https://stackoverflow.com/questions/13826803/calling-javascript-function-from-a-c-callback-in-v8/28554065#28554065
    Local<Function>::New(isolate, work->g_cb)->Call(isolate->GetCurrentContext()->Global(), 0, nullptr);
    uv_queue_work(uv_default_loop(),&work->request,WorkAsync,WorkAsyncComplete);
    // Free up the persistent function callback
    //work->g_cb.Reset();
    //delete work;
}

void CalculateResultsAsync(const v8::FunctionCallbackInfo<v8::Value>&args) {
    Isolate* isolate = args.GetIsolate();
    Local<Function> callback = Local<Function>::Cast(args[0]);
    printf("CalculateResultsAsync isolate %p Isolate::GetCurrent() %p\n", isolate, Isolate::GetCurrent());
    Work * work = new Work();
    work->request.data = work;

    // store the callback from JS in the work package so we can
    // invoke it later
    work->g_cb.Reset(isolate, callback);

    // kick of the worker thread
    //uv_queue_work(uv_default_loop(),&work->request,WorkAsync,WorkAsyncComplete);

    args.GetReturnValue().Set(Undefined(isolate));
}

void onCallback(uv_async_t* handle)
{
    Isolate * isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate);
    printf("WorkAsyncComplete isolate %p\n", isolate);
    printf("i am callback\n");
    PBSS_INFO pinfo = intf->GetCurBssInfo();
    printf("ssid %s\n", pinfo->ssid);
    Local<Function>::New(isolate, work->g_cb)->Call(isolate->GetCurrentContext()->Global(), 0, nullptr);
}

void testAsyncUV(const v8::FunctionCallbackInfo<v8::Value>&args)
{
    Isolate* isolate = args.GetIsolate();
    uv_async_init(uv_default_loop(), &s_async, onCallback);
    args.GetReturnValue().Set(Undefined(isolate));
}

void Init(Handle<Object> exports) {
    NODE_SET_METHOD(exports, "testWlan", testWlan);
    NODE_SET_METHOD(exports, "RegistCallBackAsync", RegistCallBackAsync);
    NODE_SET_METHOD(exports, "deInit", deInit);
    NODE_SET_METHOD(exports, "calculate_results_async", CalculateResultsAsync);
    NODE_SET_METHOD(exports, "testAsyncUV", testAsyncUV);
}

NODE_MODULE(wlan, Init)
