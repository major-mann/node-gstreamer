#include "main_loop.h"
#include "../threading.h"

using namespace nstr;

MainLoop* MainLoop::Singleton = MainLoop::Default();

MainLoop::MainLoop(GMainLoop* loop) : LinkedObjectBase<MainLoop, GMainLoop>(loop, MainLoop::LoopRefWrap, MainLoop::LoopUnrefWrap) {
	V8Dispatcher = NULL;
}

MainLoop::~MainLoop() {
	Stop();
}

MainLoop* MainLoop::Create(GMainLoop* loop) {
	return LinkedObjectBase<MainLoop, GMainLoop>::Create(loop);
}

MainLoop* MainLoop::Default() {
	GMainLoop* loop = g_main_loop_new (NULL, FALSE);
	return Create(loop);
}
void MainLoop::InitializeV8Instance(Handle<Object> instance) {
	LinkedObjectBase<MainLoop, GMainLoop>::InitializeV8Instance(instance);
	Local<External> data = External::New(Isolate::GetCurrent(), this);
	Helper::SetFunctionProp(instance, "run", Run, data);
	Helper::SetFunctionProp(instance, "stop", Stop, data);
	Helper::SetAccessorProp(instance, "running", RunningGetter, data);
}

void MainLoop::Run(Local<Function> complete) {
	if (V8Dispatcher == NULL) {

		std::vector<EventData*> rows;
        NSTR_MUTEX_LOCK(&mutex)
        rows.swap(EventQueue);
        NSTR_MUTEX_UNLOCK(&mutex)

        for (unsigned int i = 0, size = rows.size(); i < size; i++) {
        	rows[i]->callback(rows[i]->data);
        	delete rows[i];
    	}

		V8Dispatcher = new Async<EventData, MainLoop>(this, DispatcherCall);
		if (!complete.IsEmpty()) {
			CompleteCallback.Reset(Isolate::GetCurrent(), complete);
		}

		this->Ref();
		AsyncRunner<MainLoop, void>::Run(RunWorker, RunComplete, this);
	}
}

void* MainLoop::RunWorker(MainLoop* mloop) {
	if (mloop->V8Dispatcher != NULL) {
		g_main_loop_run(mloop->GetObject());
	}
	return NULL;
}

void MainLoop::RunComplete(MainLoop* loop, void* result) {
	if (!loop->CompleteCallback.IsEmpty()) {
		Local<Function> func = Local<Function>::New(Isolate::GetCurrent(), loop->CompleteCallback);
		func->Call(loop->GetV8Instance(), 0, new Handle<Value>[0]);
	}

	//Clean up
	loop->Stop();
	loop->Unref();
}

void MainLoop::Stop() {
	if (V8Dispatcher != NULL) {
		if (!CompleteCallback.IsEmpty()) {
			CompleteCallback.Reset();
		}
		V8Dispatcher->finish();
		V8Dispatcher = NULL;
		g_main_loop_quit(GetObject());
	}
}

void MainLoop::SendV8Event(V8ThreadCallback callback, void* data) {
	EventData* ed = new EventData();
	ed->callback = callback;
	ed->data = data;
	if (V8Dispatcher == NULL) {
		NSTR_MUTEX_LOCK(&mutex);
        EventQueue.push_back(ed);
        NSTR_MUTEX_UNLOCK(&mutex)
	} else {
		V8Dispatcher->send(ed);
	}
}

void MainLoop::DispatcherCall(MainLoop* loop, EventData* data) {
	//Just call to the user defined callback
	data->callback(data->data);

	//Delete the baton
	delete data;
}

MainLoop* MainLoop::Instance(Local<Value> value) {
	Local<External> ext = Local<External>::Cast(value);
	return (MainLoop*)ext->Value();
}

void MainLoop::Run(const FunctionCallbackInfo<Value>& args) {
	MainLoop* mloop = Instance(args.Data());
	Local<Function> callback;
	if (args.Length() > 0 && args[0]->IsFunction()) {
		callback = Local<Function>::Cast(args[0]);
	}
	mloop->Run(callback);
}
void MainLoop::Stop(const FunctionCallbackInfo<Value>& args) {
	MainLoop* mloop = Instance(args.Data());
	mloop->Stop();
}

void MainLoop::RunningGetter(Local<String> prop, const PropertyCallbackInfo<Value>& info) {
	MainLoop* self = Instance(info.Data());
    bool running = g_main_loop_is_running(self->GetObject());
    info.GetReturnValue().Set(Boolean::New(Isolate::GetCurrent(), running));
}

void MainLoop::LoopRefWrap(GMainLoop* loop) {
	g_main_loop_ref(loop);
}
void MainLoop::LoopUnrefWrap(GMainLoop* loop) {
	g_main_loop_unref(loop);
}