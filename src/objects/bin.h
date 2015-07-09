#ifndef NSTR_BIN_H
#define NSTR_BIN_H

#include "element_base.h"
#include "element.h"
#include "pad.h"

using namespace v8;
namespace nstr {

	template <class T, class CT>
	class Bin : public ElementBase<T, CT> {
		public:
			/** Adds an element to the bin */
			bool Add(Element* ele) {
				return gst_bin_add(GST_BIN(this->GetObject()), ele->GetObject());
			}

			/** Removes an element from the bin */
			bool Remove(Element* ele) {
				return gst_bin_remove(GST_BIN(this->GetObject()), ele->GetObject());
			}

			/** Finds the element with the given name in the bin */
			Element* ByName(const char* name) {
				GstElement* gele = gst_bin_get_by_name(GST_BIN(this->GetObject()), name);
				Element* ele = Element::Create(gele);
				return ele;
			}

			/** Finds the next unlinked pad in the specified direction */
			Pad* UnlinkedPad(const char* direction) {
				GstPadDirection direc;
				if (strcmp(direction, "sink") == 0) {
					direc = GST_PAD_SINK;
				} else if (strcmp(direction, "src") == 0) {
					direc = GST_PAD_SRC;
				} else {
					direc = GST_PAD_UNKNOWN;
				}

				GstPad* pad = gst_bin_find_unlinked_pad(GST_BIN(this->GetObject()), direc);
				return Pad::Create(pad);
			}

			/** Returns all the elements in the bin */
			Handle<Array> Elements() {
				//TODO: Not sure if this increases the refcount on the objects iterated?
				GstIterator* it = gst_bin_iterate_elements(GST_BIN(this->GetObject()));
				return ElementIteratorToArray(it);
			}

			/** Returns all the source elements in the bin */
			Handle<Array> Sources() {
				//TODO: Not sure if this increases the refcount on the objects iterated?
				GstIterator* it = gst_bin_iterate_sources(GST_BIN(this->GetObject()));
				return ElementIteratorToArray(it);
			}

			/** Returns all the sink elements in the bin */
			Handle<Array> Sinks() {
				//TODO: Not sure if this increases the refcount on the objects iterated?
				GstIterator* it = gst_bin_iterate_sinks(GST_BIN(this->GetObject()));
				return ElementIteratorToArray(it);
			}

			void Walk(Handle<Function> handler) {
				//TODO: Not sure if this increases the refcount on the objects iterated?
				GstIterator* it = gst_bin_iterate_recurse(GST_BIN(this->GetObject()));
				GstIteratorResult res;
				GValue elem;
				while ((res = gst_iterator_next(it, &elem)) == GST_ITERATOR_OK) {
					GstElement* ge = (GstElement*)g_value_peek_pointer(&elem);
					Element* e = Element::Create(ge);
					bool result = Helper::RunFilterFunction(handler, e->GetV8Instance());

					//User can cancel by returning falsy
					if (!result) {
						break;
					}
				}
				gst_iterator_free(it);
			}
		protected:
			Bin(CT* obj) : ElementBase<T, CT>(obj) { }

			/**
            * Adds the members to the instance
            */
			void InitializeV8Instance(Handle<Object> instance) {
				ElementBase<T, CT>::InitializeV8Instance(instance);

            	Local<External> data = External::New(Isolate::GetCurrent(), this);
            	Helper::SetFunctionProp(instance, "add", Add, data);
            	Helper::SetFunctionProp(instance, "remove", Remove, data);
            	Helper::SetFunctionProp(instance, "byName", ByName, data);
            	Helper::SetFunctionProp(instance, "unlinkedPad", UnlinkedPad, data);
            	Helper::SetFunctionProp(instance, "elements", Elements, data);
            	Helper::SetFunctionProp(instance, "sources", Sources, data);
            	Helper::SetFunctionProp(instance, "sinks", Sinks, data);
            	Helper::SetFunctionProp(instance, "walk", Walk, data);
            };


		private:
			/** Reads all elements off the iterator, converts them, and places them into a return array */
			Handle<Array> ElementIteratorToArray(GstIterator* it) {
				GstIteratorResult res;
				GValue elem;
				Local<Array> arr = Array::New(Isolate::GetCurrent());
				int cnt = 0;
				while ((res = gst_iterator_next(it, &elem)) == GST_ITERATOR_OK) {
					GstElement* ge = (GstElement*)g_value_peek_pointer(&elem);
					Element* e = Element::Create(ge);
					arr->Set(cnt, e->GetV8Instance());
					cnt++;
				}
				gst_iterator_free(it);
				return arr;
			}

			static Bin* Instance(Local<Value> value) {
				Local<External> ext = Local<External>::Cast(value);
				return (Bin*)ext->Value();
			}

			static void Add(const FunctionCallbackInfo<Value>& args) {
				Element* add = NULL;
				if (args.Length() > 0) {
					add = Element::Unwrap(args[0]);
				}
				if (add == NULL) {
					Helper::ThrowV8Error("add MUST be supplied with an element");
				} else {
					Bin* instance = Instance(args.Data());
					bool result = instance->Add(add);
					args.GetReturnValue().Set(Helper::ToBooleanValue(result));
				}
			}

			static void Remove(const FunctionCallbackInfo<Value>& args) {
				Element* rem = NULL;
				if (args.Length() > 0) {
					rem = Element::Unwrap(args[0]);
				}
				if (rem == NULL) {
					Helper::ThrowV8Error("remove MUST be supplied with an element");
				} else {
					Bin* instance = Instance(args.Data());
					bool result = instance->Remove(rem);
					args.GetReturnValue().Set(Helper::ToBooleanValue(result));
				}
			}
			static void ByName(const FunctionCallbackInfo<Value>& args) {
				const char* name = NULL;
				if (args.Length() > 0 && args[0]->IsString()) {
					String::Utf8Value uval(args[0]->ToString());
					name = *uval;
				}
				if (name == NULL) {
					Helper::ThrowV8Error("byName MUST be supplied with a name");
				} else {
					Bin* instance = Instance(args.Data());
					Element* ele = instance->ByName(name);
					if (ele == NULL) {
						args.GetReturnValue().Set(Null(Isolate::GetCurrent()));
					} else {
						args.GetReturnValue().Set(ele->GetV8Instance());
					}
				}
			}
			static void UnlinkedPad(const FunctionCallbackInfo<Value>& args) {
				const char* direction = "unknown";

				if (args.Length() > 0 && args[0]->IsString()) {
					String::Utf8Value uval(args[0]->ToString());
					direction = *uval;
				}

				Bin* instance = Instance(args.Data());
				Pad* unlinked = instance->UnlinkedPad(direction);
				if (unlinked == NULL) {
					args.GetReturnValue().Set(Null(Isolate::GetCurrent()));
				} else {
					args.GetReturnValue().Set(unlinked->GetV8Instance());
				}
			}
			static void Elements(const FunctionCallbackInfo<Value>& args) {
				Bin* instance = Instance(args.Data());
				Local<Array> elements = instance->Elements();
				args.GetReturnValue().Set(elements);
			}
			static void Sources(const FunctionCallbackInfo<Value>& args) {
				Bin* instance = Instance(args.Data());
				Local<Array> elements = instance->Sources();
				args.GetReturnValue().Set(elements);
			}
			static void Sinks(const FunctionCallbackInfo<Value>& args) {
				Bin* instance = Instance(args.Data());
				Local<Array> elements = instance->Sinks();
				args.GetReturnValue().Set(elements);
			}
			static void Walk(const FunctionCallbackInfo<Value>& args) {
				if (args.Length() > 0 && args[0]->IsFunction()) {
					Local<Function> handler = Local<Function>::Cast(args[0]);
					Bin* instance = Instance(args.Data());
					instance->Walk(handler);
				} else {
					Helper::ThrowV8Error("walk MUST be supplied with a handler function");
				}
			}
	};

}

#endif













