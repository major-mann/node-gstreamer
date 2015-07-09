#ifndef NSTR_ELEMENT_BASE_H
#define NSTR_ELEMENT_BASE_H

#include "linked_object_base.h"
#include "g_object_wrap.h"
#include "pad.h"
#include "async.h"
#include "main_loop.h"

//TODO: The object never gets destroyed, so async never gets destroyed.
//  starting to think the async needs to be tied to the loop code....

namespace nstr {

    template <class T, class CT>
    class ElementBase : public LinkedObjectBase<T, CT> {
        public:
            static T* Create(CT* ele) {
                T* result = LinkedObjectBase<T, CT>::Create(ele);
                return result;
            }

            bool Link(ElementBase<T, CT>* dest) {
                CT* src = this->GetObject();
                CT* dst = dest->GetObject();

                //TODO: Getting massive errors here?
                bool result = gst_element_link(src, dst);
                return result;
            }
            bool LinkBack(ElementBase<T, CT>* src) {
                return gst_element_link(src->GetObject(), this->GetObject());
            }

            void Unlink(ElementBase<T, CT>* dest) {
                gst_element_unlink(this->GetObject(), dest->GetObject());
            }
            void UnlinkBack(ElementBase<T, CT>* src) {
                gst_element_unlink(src->GetObject(), this->GetObject());
            }


            /**
            * Wraps gst_element_get_static_pad to return the (already existing) named pad
            */
            Pad* StaticPad(const char* name) {
                GstPad* gpad = gst_element_get_static_pad(this->GetObject(), name);
                Pad* pad = Pad::Create(gpad);
                return pad;
            }

            //NOTE: THESE MUST BE CALLED ASYNCRONOUSLY WITH CALLBACKS!!
            const char* GetState() {
                GstState state;
                GstState pending;
                GstStateChangeReturn scr = gst_element_get_state(
                    this->GetObject(),
                    &state,
                    &pending,
                    //20 Seconds.. TODO: Move this to a constant, or perhaps global config?
                    //  maybe even element config?
                    1000 * 1000 * 20); 


                switch (scr) {
                    case  GST_STATE_CHANGE_SUCCESS:
                        return Helper::StateToString(state);
                    case GST_STATE_CHANGE_FAILURE:
                        printf("%s\n", "State change failure!");
                        return NULL;
                    case GST_STATE_CHANGE_ASYNC:
                    case GST_STATE_CHANGE_NO_PREROLL:
                        return Helper::StateToString(pending);
                    default:
                        printf("%s\n", "INVALID PROGRAM! UNEXPECTED RETURN!");
                        exit(1);
                        break;
                }
            }
            const char* SetState(const char* state) {
                GstStateChangeReturn result;
                if (std::strcmp(state, "void") == 0) {
                    result = gst_element_set_state(this->GetObject(), GST_STATE_VOID_PENDING);
                } else if (std::strcmp(state, "null") == 0) {
                    result = gst_element_set_state(this->GetObject(), GST_STATE_NULL);
                } else if (std::strcmp(state, "ready") == 0) {
                    result = gst_element_set_state(this->GetObject(), GST_STATE_READY);
                } else if (std::strcmp(state, "paused") == 0) {
                    result = gst_element_set_state(this->GetObject(), GST_STATE_PAUSED);
                } else if (std::strcmp(state, "playing") == 0) {
                    result = gst_element_set_state(this->GetObject(), GST_STATE_PLAYING);
                } else {
                    printf("Unrecognized state: \"%s\"\n", state);
                    return NULL;
                }

                switch (result) {
                    case GST_STATE_CHANGE_SUCCESS:
                    case GST_STATE_CHANGE_ASYNC:
                    case GST_STATE_CHANGE_NO_PREROLL:
                        return GetState();
                    case GST_STATE_CHANGE_FAILURE:
                        return NULL;
                    default:
                        printf("%s\n", "Critical application error unrecognized GstStateChangeReturn element_base.h line ~115");
                }
            }

            long Position(GstFormat format) {
                long position = 0;
                if (!gst_element_query_position(this->GetObject(), format, &position)) {
                    position = -1;
                }
                return position;
            }
            long Duration(GstFormat format) {
                long duration = 0;
                if (!gst_element_query_duration(this->GetObject(), format, &duration)) {
                    duration = -1;
                }
                return duration;
            }

        protected:
            ElementBase(CT* element) 
                : LinkedObjectBase<T, CT>(element, Helper::GstRefWrap, Helper::GstUnrefWrap) {

                SignalIdPadAdded = 0;
                SignalIdPadRemoved = 0;
                SignalIdNoMorePads = 0;
                Settings = GObjectWrap::Create((GObject*)element);

                OnObjectUpdated(NULL, element);
            }
            void InitializeV8Instance(Handle<Object> instance) {
                LinkedObjectBase<T, CT>::InitializeV8Instance(instance);

                Local<External> data = External::New(Isolate::GetCurrent(), this);
                Helper::SetFunctionProp(instance, "state", GetState, data);
                Helper::SetFunctionProp(instance, "setState", SetState, data);
                Helper::SetFunctionProp(instance, "link", Link, data);
                Helper::SetFunctionProp(instance, "linkBack", LinkBack, data);
                Helper::SetFunctionProp(instance, "unlink", Unlink, data);
                Helper::SetFunctionProp(instance, "unlinkBack", UnlinkBack, data);
                Helper::SetFunctionProp(instance, "staticPad", StaticPad, data);
                Helper::SetFunctionProp(instance, "position", Position, data);
                Helper::SetFunctionProp(instance, "duration", Duration, data);
                Helper::SetAccessorProp(instance, "settings", GetSettingsAccessor, data);

            }
            void OnObjectUpdated(CT* ov, CT* nv) {
                if (ov != NULL) {
                    if (SignalIdPadAdded != 0) {
                        g_signal_handler_disconnect(ov, SignalIdPadAdded);
                        SignalIdPadAdded = 0;
                    }

                    if (SignalIdPadRemoved != 0) {
                        g_signal_handler_disconnect(ov, SignalIdPadRemoved);
                        SignalIdPadRemoved = 0;
                    }

                    if (SignalIdNoMorePads != 0) {
                        g_signal_handler_disconnect(ov, SignalIdNoMorePads);
                        SignalIdNoMorePads = 0;
                    }
                }

                if (nv != NULL) {
                    SignalIdPadAdded = g_signal_connect (nv, "pad-added", G_CALLBACK (OnPadAdded), this);
                    SignalIdPadRemoved = g_signal_connect (nv, "pad-removed", G_CALLBACK (OnPadRemoved), this);
                    SignalIdNoMorePads = g_signal_connect (nv, "no-more-pads", G_CALLBACK (OnNoMorePads), this);
                }
            }

        private:
            struct StateData {
                ElementBase<T, CT>* self;
                Persistent<Function> callback;
                const char* state;
            };

            struct EventData {
                const char* name;
                GstPad* pad;
                gulong probeId;
                ElementBase<T, CT>* self;
            };

            GObjectWrap* Settings;
            gulong SignalIdPadAdded;
            gulong SignalIdPadRemoved;
            gulong SignalIdNoMorePads;

            static ElementBase<T, CT>* Instance(Local<Value> value) {
                Local<External> ext = Local<External>::Cast(value);
                return (ElementBase<T, CT>*)ext->Value();
            }

            static void OnV8EventRaised(void* data) {
                EventData* ed = (EventData*)data;
                if (strcmp(ed->name, "pad-added") == 0) {
                    Pad* pad = Pad::Create(ed->pad);
                    Handle<Value> argv[1];
                    argv[0] = pad->GetV8Instance();
                    ed->self->Emit("pad-added", 1, argv);
                    if (ed->probeId > 0) {
                        //Remove the probe which allows the processing to continue.
                        gst_pad_remove_probe(ed->pad, ed->probeId);
                    }
                } else if (strcmp(ed->name, "pad-removed") == 0) {
                    Pad* pad = Pad::Create(ed->pad);
                    Handle<Value> argv[1];
                    argv[0] = pad->GetV8Instance();
                    ed->self->Emit("pad-removed", 1, argv);
                } else if (strcmp(ed->name, "no-more-pads") == 0) {
                    Handle<Value> argv[0];
                    ed->self->Emit("no-more-pads", 0, argv);
                }
                delete ed;
            }

            static void RaiseV8Event(ElementBase<T, CT>* self, const char* name, GstPad* pad, gulong probeId) {
                EventData* ed = new EventData();
                ed->name = name;
                ed->pad = pad;
                ed->probeId = probeId;
                ed->self = self;
                MainLoop::Singleton->SendV8Event(&OnV8EventRaised, ed);
            }

            static GstPadProbeReturn OnPadProbed(GstPad* pad, GstPadProbeInfo* info, gpointer userData) {
                //We want to keep blocking until we manually remove the probe (after we have called the relevant event)
                //  in javascript.
                return GST_PAD_PROBE_OK;
            }

            static void OnNoMorePads(CT *gstelement, gpointer user_data) {
                ElementBase<T, CT>* self = (ElementBase<T, CT>*)user_data;
                RaiseV8Event(self, "no-more-pads", NULL, 0);
            }
            static void OnPadAdded(CT *gstelement, GstPad* gpad, gpointer user_data) {
                ElementBase<T, CT>* self = (ElementBase<T, CT>*)user_data;
                gulong probeId = 0;

                //Block until the event has been raised in javascript
                probeId = gst_pad_add_probe(gpad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM, OnPadProbed, self, NULL);
                RaiseV8Event(self, "pad-added", gpad, probeId);
            }
            static void OnPadRemoved(CT *gstelement, GstPad* gpad, gpointer user_data) {
                ElementBase<T, CT>* self = (ElementBase<T, CT>*)user_data;
                RaiseV8Event(self, "pad-removed", gpad, 0);
            }

            static void GetSettingsAccessor(Local<String> prop, const PropertyCallbackInfo<Value>& info) {
                ElementBase<T, CT>* self = Instance(info.Data());
                GObjectWrap* settings = self->Settings;
                if (settings == NULL) {
                    //This should not happen
                    printf("%s\n", "CRITICAL! SETTINGS OBJECT NULL!");
                    exit(-1);
                    return;
                }
                info.GetReturnValue().Set(settings->GetV8Instance());
            }

            static void GetState(const FunctionCallbackInfo<Value>& args) {
                if (args.Length() > 0 && args[0]->IsFunction()) {
                    ElementBase<T, CT>* self = Instance(args.Data());
                    Local<Function> func = Local<Function>::Cast(args[0]);

                    StateData* data = new StateData();
                    data->self = self;
                    data->callback.Reset(Isolate::GetCurrent(), func);
                    data->state = NULL;
                    self->Ref();
                    AsyncRunner<StateData, const char>::Run(GetStateWorker, GetStateCallback, data);
                } else {
                    Helper::ThrowV8Error("state function expects a function  paramter to be called once the state has been retrieved");
                }
            }
            static const char* GetStateWorker(StateData* data) {
                const char* result = data->self->GetState();
                return result;
            }
            static void GetStateCallback(StateData* data, const char* state) {
                Local<Function> func = Local<Function>::New(Isolate::GetCurrent(), data->callback);
                Handle<Value> argv[1];
                argv[0] = Helper::ToValue(state);

                data->self->Unref();
                data->callback.Reset();
                delete data;

                //Callback with the result.
                func->Call(data->self->GetV8Instance(), 1, argv);
            }

            static void SetState(const FunctionCallbackInfo<Value>& args) {
                if (args.Length() > 1 && args[0]->IsString() && args[1]->IsFunction()) {
                    ElementBase<T, CT>* self = Instance(args.Data());
                    String::Utf8Value uval(args[0]->ToString());
                    char* state = new char[std::strlen(*uval)];
                    std::strcpy(state, *uval);
                    Local<Function> func = Local<Function>::Cast(args[1]);
                    StateData* data = new StateData();
                    data->self = self;
                    data->callback.Reset(Isolate::GetCurrent(), func);
                    data->state = state;
                    self->Ref();
                    AsyncRunner<StateData, const char>::Run(SetStateWorker, SetStateCallback, data);
                } else {
                    Helper::ThrowV8Error("setState function expects string parameter (state name) followed by a function paramter to be called once the state has been retrieved");
                }
            }
            static const char* SetStateWorker(StateData* data) {
                const char* result = data->self->SetState(data->state);
                return result;
            }
            static void SetStateCallback(StateData* data, const char* state) {
                Isolate* isolate = Isolate::GetCurrent();
                if (state == NULL) {
                    Helper::ThrowV8Error("Unable to set state");
                } else {
                    Local<Function> func = Local<Function>::New(isolate, data->callback);
                    Handle<Value> argv[1];

                    argv[0] = Helper::ToValue(state);
                    data->self->Unref();
                    data->callback.Reset();
                    delete data->state; //This will delete the supplied state string
                    delete data;
                    //Callback with the result.
                    func->Call(data->self->GetV8Instance(), 1, argv);
                }
            }

            static void Link(const FunctionCallbackInfo<Value>& args) {
                ElementBase<T, CT>* other = NULL;
                if (args.Length() > 0 && args[0]->IsObject()) {
                    other = node::ObjectWrap::Unwrap<T>(args[0]->ToObject());
                }
                if (other == NULL) {
                    Helper::ThrowV8Error("link function MUST be supplied an element as an argument");
                } else {
                    ElementBase<T, CT>* instance = Instance(args.Data());
                    bool result = instance->Link(other);
                    args.GetReturnValue().Set(Helper::ToBooleanValue(result));
                }
            }
            static void LinkBack(const FunctionCallbackInfo<Value>& args) {
                ElementBase<T, CT>* other = NULL;
                if (args.Length() > 0 && args[0]->IsObject()) {
                    other = node::ObjectWrap::Unwrap<T>(args[0]->ToObject());
                }
                if (other == NULL) {
                    Helper::ThrowV8Error("linkBack function MUST be supplied an element as an argument");
                } else {
                    ElementBase<T, CT>* instance = Instance(args.Data());
                    bool result = instance->LinkBack(other);
                    args.GetReturnValue().Set(Helper::ToBooleanValue(result));
                }
            }
            static void Unlink(const FunctionCallbackInfo<Value>& args) {
                ElementBase<T, CT>* other = NULL;
                if (args.Length() > 0 && args[0]->IsObject()) {
                    other = node::ObjectWrap::Unwrap<T>(args[0]->ToObject());
                }
                if (other == NULL) {
                    Helper::ThrowV8Error("unlink function MUST be supplied an element as an argument");
                } else {
                    ElementBase<T, CT>* instance = Instance(args.Data());
                    instance->Unlink(other);
                }
            }
            static void UnlinkBack(const FunctionCallbackInfo<Value>& args) {
                ElementBase<T, CT>* other = NULL;
                if (args.Length() > 0 && args[0]->IsObject()) {
                    other = node::ObjectWrap::Unwrap<T>(args[0]->ToObject());
                }
                if (other == NULL) {
                    Helper::ThrowV8Error("unlinkBack function MUST be supplied an element as an argument");
                } else {
                    ElementBase<T, CT>* instance = Instance(args.Data());
                    instance->UnlinkBack(other);
                }
            }

            static void StaticPad(const FunctionCallbackInfo<Value>& args) {
                if (args.Length() > 0 && args[0]->IsString()) {
                    ElementBase<T, CT>* instance = Instance(args.Data());
                    String::Utf8Value uval(args[0]);
                    Pad* pad = instance->StaticPad(*uval);
                    args.GetReturnValue().Set(pad->GetV8Instance());
                } else {
                    Helper::ThrowV8Error("name MUST be supplied when calling staticPad");
                }
            }

            static GstFormat ValueToFormat(Handle<Value> value) {
                GstFormat fmt = GST_FORMAT_DEFAULT;
                if (!value.IsEmpty() && value->IsString()) {
                    String::Utf8Value sfmt(value->ToString());
                    if (strcmp(*sfmt, "bytes") == 0) {
                        fmt = GST_FORMAT_BYTES;
                    } else if (strcmp(*sfmt, "time") == 0) {
                        fmt = GST_FORMAT_TIME;
                    } else if (strcmp(*sfmt, "buffers") == 0) {
                        fmt = GST_FORMAT_BUFFERS;
                    } else if (strcmp(*sfmt, "percent") == 0) {
                        fmt = GST_FORMAT_PERCENT;
                    }
                }
                return fmt;
            }

            static void Position(const FunctionCallbackInfo<Value>& args) {
                GstFormat fmt = ValueToFormat(args.Length() > 0 ? args[0] : Handle<Value>());
                ElementBase<T, CT>* self = Instance(args.Data());
                unsigned long result = self->Position(fmt);
                args.GetReturnValue().Set(Helper::ToValue(result));
            }

            static void Duration(const FunctionCallbackInfo<Value>& args) {
                GstFormat fmt = ValueToFormat(args.Length() > 0 ? args[0] : Handle<Value>());
                ElementBase<T, CT>* self = Instance(args.Data());
                unsigned long result = self->Duration(fmt);
                args.GetReturnValue().Set(Helper::ToValue(result));
            }
    };

}

#endif
