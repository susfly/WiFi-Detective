#include <node.h>
#include <windows.h>
#include <wlanapi.h>

#pragma comment(lib, "wlanapi.lib")

namespace demo {

using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::String;
using v8::Value;

typedef int (* lpFuna)(int);
// This is the implementation of the "add" method
// Input arguments are passed using the
// const FunctionCallbackInfo<Value>& args struct
void Add(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  // Check the number of arguments passed.
  if (args.Length() < 2) {
    // Throw an Error that is passed back to JavaScript
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong number of arguments")));
    return;
  }

  // Check the argument types
  if (!args[0]->IsNumber() || !args[1]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }
  //WlanScan();
  // Perform the operation
  double value = args[0]->NumberValue() + args[1]->NumberValue();


  HINSTANCE hDll;
  lpFuna Funa;
  hDll = LoadLibrary("./testdll.dll");
  int err_no = GetLastError();
  printf("hDll:%p err_no %d\n", hDll, err_no);
  if (hDll != NULL) {
      Funa = (lpFuna)GetProcAddress(hDll,"test_dll");
      if(Funa != NULL) {
          value = Funa(2);
          printf("call add in dll:%f\n",value);
      }
      FreeLibrary(hDll);
  }

  Local<Number> num = Number::New(isolate, value);

  // Set the return value (using the passed in
  // FunctionCallbackInfo<Value>&)
  args.GetReturnValue().Set(num);
}

void Init(Local<Object> exports) {
  NODE_SET_METHOD(exports, "add", Add);
}

NODE_MODULE(addon, Init)
}  // namespace demo