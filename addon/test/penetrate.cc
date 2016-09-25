#include <node.h>
#include <v8.h>
#include <windows.h>
#include <dwmapi.h>
#pragma comment (lib , "dwmapi.lib" )

using namespace v8;

void penetrate(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	v8::String::Utf8Value str(args[0]->ToString());
	char *name = *str;
	HWND h = FindWindow(NULL,(LPCSTR)name);
	if (h != NULL) {
		BOOL bDwm ;
    DwmIsCompositionEnabled (&bDwm);
    if (bDwm )
    {
        MARGINS mrg = {-1};
        HRESULT hr = DwmExtendFrameIntoClientArea(h , &mrg);
				printf("call maoboli %d\n", SUCCEEDED(hr));
    }
		//int extendedStyle = GetWindowLong(h, GWL_EXSTYLE);
		//SetWindowLong(h, GWL_EXSTYLE, extendedStyle | WS_EX_TRANSPARENT | WS_EX_LAYERED);
		args.GetReturnValue().Set(true);
		return;
	}
	args.GetReturnValue().Set(false);
}

void Init(Handle<Object> exports) {
	Isolate* isolate = Isolate::GetCurrent();
	exports->Set(String::NewFromUtf8(isolate, "penetrate"),
		FunctionTemplate::New(isolate, penetrate)->GetFunction());
}

NODE_MODULE(penetrate, Init)
