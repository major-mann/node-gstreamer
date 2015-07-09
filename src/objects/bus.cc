#include "bus.h"
#include "main_loop.h"
#include "tag_list.h"
#include "clock.h"

using namespace nstr;

Bus::Bus(GstBus* bus) : 
	LinkedObjectBase<Bus, GstBus>(bus, Helper::GstRefWrap, Helper::GstUnrefWrap)
{
	WatchId = 0;
	Priority = G_PRIORITY_DEFAULT;
}

Bus* Bus::Create(GstBus* bus) {
	return LinkedObjectBase<Bus, GstBus>::Create(bus);
}

uint Bus::Watch() {
	if (WatchId == 0) {
		GstBusFunc busFunc = &BusCall;
		WatchId = gst_bus_add_watch_full(
			GetObject(),
			Priority,
			busFunc,
			this,
			NULL);
	}
	return WatchId;
}

bool Bus::ClearWatch() {
	bool result = gst_bus_remove_watch(GetObject());
	WatchId = 0;
	return result;
}

int Bus::GetPriority() {
	return Priority;
}
void Bus::SetPriority(int priority) {
	Priority = priority;
	ClearWatch();
	Watch();
}

void Bus::InitializeV8Instance(Handle<Object> instance) {
	LinkedObjectBase<Bus, GstBus>::InitializeV8Instance(instance);
	Local<External> data = External::New(Isolate::GetCurrent(), this);
	Helper::SetFunctionProp(instance, "watch", &Watch, data);
	Helper::SetFunctionProp(instance, "clearWatch", &ClearWatch, data);
	Helper::SetAccessorProp(instance, "priority", &GetPriorityAccessor, &SetPriorityAccessor, data);
}

gboolean Bus::BusCall(GstBus* bus, GstMessage* msg, gpointer data) {
	Bus* self = (Bus*)data;
	WatchData* wd = new WatchData();
	wd->self = self;
	//Ref the message so we don't loose it
	gst_message_ref(msg);
	wd->message = msg;
	MainLoop::Singleton->SendV8Event(&WatchV8Callback, wd);

	//We will only ever manually remove the watch
	return TRUE;
}

void Bus::WatchV8Callback(void* data) {
	WatchData* wd = (WatchData*)data;

	GstMessageType mt = GST_MESSAGE_TYPE(wd->message);
	switch (mt) {
		case GST_MESSAGE_UNKNOWN: {
				Handle<Value> argv[] = { String::NewFromUtf8(Isolate::GetCurrent(), "unknown") };
				wd->self->Emit("watch", 1, argv);
			}
			break;
		case GST_MESSAGE_EOS: {
				Handle<Value> argv[] = { String::NewFromUtf8(Isolate::GetCurrent(), "eos") };
				wd->self->Emit("watch", 1, argv);
			}
			break;
		case GST_MESSAGE_ERROR: {
				GError* err = NULL;
				gchar* dbg = NULL;
				gst_message_parse_error(wd->message, &err, &dbg);
				Handle<Value> argv[] = { 
					String::NewFromUtf8(Isolate::GetCurrent(), "error"),
					String::NewFromUtf8(Isolate::GetCurrent(), err->message),
					String::NewFromUtf8(Isolate::GetCurrent(), dbg)
				};
				g_error_free(err);
				wd->self->Emit("watch", 3, argv);
			}
			break;
		case GST_MESSAGE_WARNING: {
				GError* err = NULL;
				gchar* dbg = NULL;
				gst_message_parse_error(wd->message, &err, &dbg);
				Handle<Value> argv[] = { 
					String::NewFromUtf8(Isolate::GetCurrent(), "warning"),
					String::NewFromUtf8(Isolate::GetCurrent(), err->message),
					String::NewFromUtf8(Isolate::GetCurrent(), dbg)
				};
				g_error_free(err);
				wd->self->Emit("watch", 3, argv);
			}
			break;
		case GST_MESSAGE_INFO: {
				GError* err = NULL;
				gchar* dbg = NULL;
				gst_message_parse_error(wd->message, &err, &dbg);
				Handle<Value> argv[] = { 
					String::NewFromUtf8(Isolate::GetCurrent(), "info"),
					String::NewFromUtf8(Isolate::GetCurrent(), err->message),
					String::NewFromUtf8(Isolate::GetCurrent(), dbg)
				};
				g_error_free(err);
				wd->self->Emit("watch", 3, argv);
			}
			break;
		case GST_MESSAGE_TAG: {
				GstTagList* gTagList = NULL;
				gst_message_parse_tag(wd->message, &gTagList);
				TagList* tagList = TagList::Create(gTagList);
				Handle<Value> argv[] = { 
					String::NewFromUtf8(Isolate::GetCurrent(), "tag"),
					tagList->GetV8Instance()
				};
				wd->self->Emit("watch", 2, argv);
			}
			break;
		case GST_MESSAGE_BUFFERING: {
				printf("%s\n", "Not implemented GST_MESSAGE_BUFFERING");
				Handle<Value> argv[] = { String::NewFromUtf8(Isolate::GetCurrent(), "buffering") };
				wd->self->Emit("watch", 1, argv);
				//TODO: "buffering", <int percent>
				//gst_message_parse_buffering
			}
			break;
		case GST_MESSAGE_STATE_CHANGED: {

				GstState ns, os;
				gst_message_parse_state_changed(wd->message, &os, &ns, NULL);

				const char* oss = Helper::StateToString(os);
				const char* nss = Helper::StateToString(ns);

				Isolate* isolate = Isolate::GetCurrent();
				Handle<Value> argv[] = { 
					String::NewFromUtf8(isolate, "state"),
					String::NewFromUtf8(isolate, nss),
					String::NewFromUtf8(isolate, oss)
				};
				wd->self->Emit("watch", 3, argv);
			}
			break;
		case GST_MESSAGE_STEP_DONE: {
				printf("%s\n", "Not implemented GST_MESSAGE_STEP_DONE");
				Handle<Value> argv[] = { String::NewFromUtf8(Isolate::GetCurrent(), "step_done") };
				wd->self->Emit("watch", 1, argv);
				/*TODO: "step", { 
					 GstFormat *format;
	                 guint64 *amount;
	                 gdouble *rate;
	                 gboolean *flush;
	                 gboolean *intermediate;
	                 guint64 *duration;
	                 gboolean *eos;
				}*/
				//gst_message_parse_step_done
			}
			break;
		case GST_MESSAGE_CLOCK_PROVIDE: {
				GstClock* gclock = NULL;
				gboolean ready = FALSE;
				gst_message_parse_clock_provide(wd->message, &gclock, &ready);
				Clock* clock = Clock::Create(gclock);
				Handle<Value> argv[] = { 
					String::NewFromUtf8(Isolate::GetCurrent(), "clock_provide"),
					clock->GetV8Instance(),
					Boolean::New(Isolate::GetCurrent(), ready)
				};
				wd->self->Emit("watch", 3, argv);
			}
			break;
		case GST_MESSAGE_CLOCK_LOST: {
				GstClock* gclock = NULL;
				gst_message_parse_clock_lost(wd->message, &gclock);
				Clock* clock = Clock::Create(gclock);
				Handle<Value> argv[] = { 
					String::NewFromUtf8(Isolate::GetCurrent(), "clock_lost"),
					clock->GetV8Instance() 
				};
				wd->self->Emit("watch", 2, argv);
			}
			break;
		case GST_MESSAGE_NEW_CLOCK: {
				GstClock* gclock = NULL;
				gst_message_parse_new_clock(wd->message, &gclock);
				Clock* clock = Clock::Create(gclock);
				Handle<Value> argv[] = { 
					String::NewFromUtf8(Isolate::GetCurrent(), "new_clock"),
					clock->GetV8Instance() 
				};
				wd->self->Emit("watch", 2, argv);
			}
			break;
		case GST_MESSAGE_STRUCTURE_CHANGE: {
				printf("%s\n", "Not implemented GST_MESSAGE_STRUCTURE_CHANGE");
				Handle<Value> argv[] = { String::NewFromUtf8(Isolate::GetCurrent(), "structure_change") };
				wd->self->Emit("watch", 1, argv);
			}
			break;
		case GST_MESSAGE_STREAM_STATUS: {
				GstStreamStatusType type;
				GstElement* ele;
				gst_message_parse_stream_status(wd->message, &type, &ele);

				const char* status;
				switch (type) {
					case GST_STREAM_STATUS_TYPE_CREATE:
						status = "create";
						break;
					case GST_STREAM_STATUS_TYPE_ENTER:
						status = "enter";
						break;
					case GST_STREAM_STATUS_TYPE_LEAVE:
						status = "leave";
						break;
					case GST_STREAM_STATUS_TYPE_DESTROY:
						status = "destroy";
						break;
					case GST_STREAM_STATUS_TYPE_START:
						status = "start";
						break;
					case GST_STREAM_STATUS_TYPE_PAUSE:
						status = "pause";
						break;
					case GST_STREAM_STATUS_TYPE_STOP:
						status = "stop";
						break;
					default:
						printf("%s\n", "Critical error! Unrecognized stream status type!");
						exit(-1);
						return;
				}
				Handle<Value> argv[] = { 
					String::NewFromUtf8(Isolate::GetCurrent(), "stream_status"),
					String::NewFromUtf8(Isolate::GetCurrent(), status) 
				};
				wd->self->Emit("watch", 2, argv);
			}
			break;
		case GST_MESSAGE_APPLICATION: {
				printf("%s\n", "Not implemented GST_MESSAGE_APPLICATION");
				Handle<Value> argv[] = { String::NewFromUtf8(Isolate::GetCurrent(), "application") };
				wd->self->Emit("watch", 1, argv);
			}
			break;
		case GST_MESSAGE_ELEMENT: {
				printf("%s\n", "Not implemented GST_MESSAGE_ELEMENT");
				Handle<Value> argv[] = { String::NewFromUtf8(Isolate::GetCurrent(), "element") };
				wd->self->Emit("watch", 1, argv);
			}
			break;
		case GST_MESSAGE_SEGMENT_START: {
				printf("%s\n", "Not implemented GST_MESSAGE_SEGMENT_START");
				Handle<Value> argv[] = { String::NewFromUtf8(Isolate::GetCurrent(), "segment_start") };
				wd->self->Emit("watch", 1, argv);
			}
			break;
		case GST_MESSAGE_SEGMENT_DONE: {
				printf("%s\n", "Not implemented GST_MESSAGE_SEGMENT_DONE");
				Handle<Value> argv[] = { String::NewFromUtf8(Isolate::GetCurrent(), "segment_done") };
				wd->self->Emit("watch", 1, argv);
			}
			break;
		case GST_MESSAGE_DURATION_CHANGED: {
				printf("%s\n", "Not implemented GST_MESSAGE_DURATION_CHANGED");
				Handle<Value> argv[] = { String::NewFromUtf8(Isolate::GetCurrent(), "duration_changed") };
				wd->self->Emit("watch", 1, argv);
			}
			break;
		case GST_MESSAGE_LATENCY: {
				printf("%s\n", "Not implemented GST_MESSAGE_LATENCY");
				Handle<Value> argv[] = { String::NewFromUtf8(Isolate::GetCurrent(), "latency") };
				wd->self->Emit("watch", 1, argv);
			}
			break;
		case GST_MESSAGE_ASYNC_START: {
				printf("%s\n", "Not implemented GST_MESSAGE_ASYNC_START");
				Handle<Value> argv[] = { String::NewFromUtf8(Isolate::GetCurrent(), "async_start") };
				wd->self->Emit("watch", 1, argv);
			}
			break;
		case GST_MESSAGE_ASYNC_DONE: {
				GstClockTime time;
				gst_message_parse_async_done(wd->message, &time);
				Handle<Value> argv[] = { Helper::ToValue(time) };
				wd->self->Emit("watch", 1, argv);
			}
			break;
		case GST_MESSAGE_REQUEST_STATE: {
				printf("%s\n", "Not implemented GST_MESSAGE_REQUEST_STATE");
				Handle<Value> argv[] = { String::NewFromUtf8(Isolate::GetCurrent(), "request_state") };
				wd->self->Emit("watch", 1, argv);
			}
			break;
		case GST_MESSAGE_STEP_START: {
				printf("%s\n", "Not implemented GST_MESSAGE_STEP_START");
				Handle<Value> argv[] = { String::NewFromUtf8(Isolate::GetCurrent(), "step_start") };
				wd->self->Emit("watch", 1, argv);
			}
			break;
		case GST_MESSAGE_QOS: {
				printf("%s\n", "Not implemented GST_MESSAGE_QOS");
				Handle<Value> argv[] = { String::NewFromUtf8(Isolate::GetCurrent(), "qos") };
				wd->self->Emit("watch", 1, argv);
			}
			break;
		case GST_MESSAGE_PROGRESS: {
				printf("%s\n", "Not implemented GST_MESSAGE_PROGRESS");
				Handle<Value> argv[] = { String::NewFromUtf8(Isolate::GetCurrent(), "message_progress") };
				wd->self->Emit("watch", 1, argv);
			}
			break;
		case GST_MESSAGE_TOC: {
				printf("%s\n", "Not implemented GST_MESSAGE_TOC");
				Handle<Value> argv[] = { String::NewFromUtf8(Isolate::GetCurrent(), "toc") };
				wd->self->Emit("watch", 1, argv);
			}
			break;
		case GST_MESSAGE_RESET_TIME: {
				printf("%s\n", "Not implemented GST_MESSAGE_RESET_TIME");
				Handle<Value> argv[] = { String::NewFromUtf8(Isolate::GetCurrent(), "reset_time") };
				wd->self->Emit("watch", 1, argv);
			}
			break;
		case GST_MESSAGE_STREAM_START: {
				Handle<Value> argv[] = { String::NewFromUtf8(Isolate::GetCurrent(), "stream_start") };
				wd->self->Emit("watch", 1, argv);
			}
			break;
		case GST_MESSAGE_NEED_CONTEXT: {
				printf("%s\n", "Not implemented GST_MESSAGE_NEED_CONTEXT");
				Handle<Value> argv[] = { String::NewFromUtf8(Isolate::GetCurrent(), "need_context") };
				wd->self->Emit("watch", 1, argv);
			}
			break;
		case GST_MESSAGE_HAVE_CONTEXT: {
				printf("%s\n", "Not implemented GST_MESSAGE_HAVE_CONTEXT");
				Handle<Value> argv[] = { String::NewFromUtf8(Isolate::GetCurrent(), "have_context") };
				wd->self->Emit("watch", 1, argv);
			}
			break;
		case GST_MESSAGE_EXTENDED: {
				printf("%s\n", "Not implemented GST_MESSAGE_EXTENDED");
				Handle<Value> argv[] = { String::NewFromUtf8(Isolate::GetCurrent(), "message_extended") };
				wd->self->Emit("watch", 1, argv);
			}
			break;
		case GST_MESSAGE_DEVICE_ADDED: {
				printf("%s\n", "Not implemented GST_MESSAGE_DEVICE_ADDED");
				Handle<Value> argv[] = { String::NewFromUtf8(Isolate::GetCurrent(), "device_added") };
				wd->self->Emit("watch", 1, argv);
			}
			break;
		case GST_MESSAGE_DEVICE_REMOVED: {
				printf("%s\n", "Not implemented GST_MESSAGE_DEVICE_REMOVED");
				Handle<Value> argv[] = { String::NewFromUtf8(Isolate::GetCurrent(), "device_removed") };
				wd->self->Emit("watch", 1, argv);
			}
			break;
		case GST_MESSAGE_STATE_DIRTY:
			printf("%s\n", "Warning: Deprecated state message received");
		default:
			printf("%s\n", "Ciritical error. Unrecognized return type");
			exit(-1);
	}
	gst_message_unref(wd->message);
	delete wd;
}

Bus* Bus::Instance(Local<Value> value) {
	Local<External> data = Local<External>::Cast(value);
	return (Bus*)data->Value();
}

void Bus::Watch(const FunctionCallbackInfo<Value>& args) {
	Bus* self = Instance(args.Data());
	uint result = self->Watch();
	args.GetReturnValue().Set(Helper::ToValue(result));
}

void Bus::ClearWatch(const FunctionCallbackInfo<Value>& args) {
	Bus* self = Instance(args.Data());
	bool result = self->ClearWatch();
	args.GetReturnValue().Set(Helper::ToBooleanValue(result));
}

void Bus::GetPriorityAccessor(Local<String> prop, const PropertyCallbackInfo<Value>& info) {
	Bus* self = Instance(info.Data());
	int result = self->GetPriority();
	info.GetReturnValue().Set(Helper::ToValue(result));
}

void Bus::SetPriorityAccessor(Local<String> prop, Local<Value> value, const PropertyCallbackInfo<void>& info) {
	if (value->IsNumber()) {
		double d = value->NumberValue();
		int priority = static_cast<int>(d);
		Bus* self = Instance(info.Data());
		self->SetPriority(priority);
	} else {
		Helper::ThrowV8Error("priority MUST be a number");
	}
	
}