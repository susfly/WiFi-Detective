#include <node.h>
#include <v8.h>
#include <iostream>
#include "Network.h"
using namespace v8;

CNetworkIf* pnetIf = NULL;

void get_network_state(const v8::FunctionCallbackInfo<v8::Value>&args)
{
  Isolate* isolate = args.GetIsolate();
  NETSTATE netstat;

  memset(&netstat, 0, sizeof(NETSTATE));
  int ret = pnetIf->GetNetState(&netstat);
  if (ret != ERROR_SUCCESS) {
    args.GetReturnValue().Set(Undefined(isolate));
    return;
  }
  Local<Object> obj = Object::New(isolate);
  Local<Number> rateMbps = Number::New(isolate, netstat.maxRateMbps);
  Local<Number> downBytes = Number::New(isolate, netstat.downBytes);
  Local<Number> upBytes = Number::New(isolate, netstat.upBytes);

  obj->Set(String::NewFromUtf8(isolate, "rateMbps"), rateMbps);
  obj->Set(String::NewFromUtf8(isolate, "downBytes"), downBytes);
  obj->Set(String::NewFromUtf8(isolate, "upBytes"), upBytes);

  args.GetReturnValue().Set(obj);
}

void get_network_info(const v8::FunctionCallbackInfo<v8::Value>&args)
{
    char buf[16];

    Isolate* isolate = args.GetIsolate();

    int ret = pnetIf->UpdateIfInfo();
    if (ret != ERROR_SUCCESS) {
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }

    UCHAR *mac = pnetIf->GetMacAddr();
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%02x%02x.%02x%02x.%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    Local<String> macStr = String::NewFromUtf8(isolate, buf);
    Local<String> desc = String::NewFromUtf8(isolate, pnetIf->GetDesc());
    Local<String> ipStr = String::NewFromUtf8(isolate, pnetIf->GetCurIpStr());
    Local<String> ipGwStr = String::NewFromUtf8(isolate, pnetIf->GetGatewayIpStr());

    Local<Object> obj = Object::New(isolate);
    obj->Set(String::NewFromUtf8(isolate, "mac"), macStr);
    obj->Set(String::NewFromUtf8(isolate, "desc"), desc);
    obj->Set(String::NewFromUtf8(isolate, "ipStr"), ipStr);
    obj->Set(String::NewFromUtf8(isolate, "ipGwStr"), ipGwStr);
    args.GetReturnValue().Set(obj);

    return;
}

void init(const v8::FunctionCallbackInfo<v8::Value>&args)
{
  Isolate* isolate = args.GetIsolate();
  Local<Number> err;

  if (!args[0]->IsString()) {
    err = Number::New(isolate, -1);
    args.GetReturnValue().Set(err);
    return;
  }

  v8::String::Utf8Value str(args[0]->ToString());
  char *guid = *str;
  pnetIf = new CNetworkIf(guid);
  int ret = pnetIf->UpdateIfInfo();
  if (ret != ERROR_SUCCESS) {
      err = Number::New(isolate, ret);
      args.GetReturnValue().Set(err);
  } else {
      err = Number::New(isolate, 0);
      args.GetReturnValue().Set(err);
  }

  return;
}

void init_module(Handle<Object> exports)
{
  NODE_SET_METHOD(exports, "getNetworkState", get_network_state);
  NODE_SET_METHOD(exports, "init", init);
  NODE_SET_METHOD(exports, "getNetworkInfo", get_network_info);
}

NODE_MODULE(network, init_module)
