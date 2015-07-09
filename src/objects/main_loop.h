#ifndef NSTR_MAINLOOP_H
#define NSTR_MAINLOOP_H

#include <node.h>
#include "linked_object_base.h"
#include "async.h"
#include <vector>

namespace nstr {

	class MainLoop : public LinkedObjectBase<MainLoop, GMainLoop> {
		friend class LinkedObjectBase<MainLoop, GMainLoop>;
		public:
			typedef void (*V8ThreadCallback)(void* data);

			struct EventData {
				V8ThreadCallback callback;
				void* data;
			};

			static MainLoop* Singleton;

			static MainLoop* Create(GMainLoop* loop);
			static MainLoop* Default();

			void Run(Local<Function> complete);
			static void* RunWorker(MainLoop* mloop);
			static void RunComplete(MainLoop* loop, void* result);
			void Stop();

			void SendV8Event(V8ThreadCallback callback, void* data);

		
		protected:
			MainLoop(GMainLoop* loop);
			~MainLoop();

			void InitializeV8Instance(Handle<Object> instance);

		private:
			NSTR_MUTEX_t
			Async<EventData, MainLoop>* V8Dispatcher;
			Persistent<Function> CompleteCallback;
			std::vector<EventData*> EventQueue;

			static void DispatcherCall(MainLoop* loop, EventData* data);
			static void LoopRefWrap(GMainLoop* loop);
			static void LoopUnrefWrap(GMainLoop* loop);

			static MainLoop* Instance(Local<Value> value);
			static void RunningGetter(Local<String> prop, const PropertyCallbackInfo<Value>& info);
			static void Run(const FunctionCallbackInfo<Value>& args);
			static void Stop(const FunctionCallbackInfo<Value>& args);

	};

}

#endif
