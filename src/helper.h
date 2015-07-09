#ifndef NSTR_HELPER_H
#define NSTR_HELPER_H

#include <node.h>
#include <gst/gst.h>
#include <string>

namespace nstr {

	typedef v8::Handle<v8::Value> (*ToV8Value)(void* val);

	struct cmp_str {
   		bool operator()(char const *a, char const *b) {
      		return std::strcmp(a, b) < 0;
   		}
	};

	class Helper {
		public:
			static void SetProp(v8::Local<v8::Object> obj, const char* propName, v8::Local<v8::Value> value);
			static void SetIntProp(v8::Local<v8::Object> obj, const char* propName, int value);
			static void SetUIntProp(v8::Local<v8::Object> obj, const char* propName, uint value);
			static void SetBoolProp(v8::Local<v8::Object> obj, const char* propName, bool value);
			static void SetUtf8Prop(v8::Local<v8::Object> obj, const char* propName, const char* value);
			static void SetFunctionProp(v8::Local<v8::Object> obj, const char* propName, v8::FunctionCallback call);
			static void SetFunctionProp(v8::Local<v8::Object> obj, const char* propName, v8::FunctionCallback call, v8::Handle<v8::Value> data);
			static void SetAccessorProp(v8::Local<v8::Object> obj, const char* propName, v8::AccessorGetterCallback getter);
			static void SetAccessorProp(v8::Local<v8::Object> obj, const char* propName, v8::AccessorGetterCallback getter, v8::AccessorSetterCallback setter);
			static void SetAccessorProp(v8::Local<v8::Object> obj, const char* propName, v8::AccessorGetterCallback getter, v8::Handle<v8::Value> data);
			static void SetAccessorProp(v8::Local<v8::Object> obj, const char* propName, v8::AccessorGetterCallback getter, v8::AccessorSetterCallback setter, v8::Handle<v8::Value> data);

			static void ThrowV8Error(const char* message);
			static void ThrowV8Error(const char* message, const char* arg1);
			static void ThrowV8Error(const char* message, const char* arg1, const char* arg2);
			static void ThrowV8Error(const char *message, int code);
			static uint AsUInt(v8::Handle<v8::Value> value);
			static uint AsUInt(v8::Handle<v8::Value> value, uint def);
			static bool RunFilterFunction(v8::Handle<v8::Function> func, v8::Handle<v8::Value> item);
			static v8::Handle<v8::Array> GListToArray(const GList* list, ToV8Value convert);
			static v8::Handle<v8::Array> GListToArray(GList* list, ToV8Value convert);
			static v8::Handle<v8::Value> ToValue(const char* str);
			//static v8::Handle<v8::Value> ToValue(unsigned long num);
			//static v8::Handle<v8::Value> ToValue(long num);
			static v8::Handle<v8::Value> ToValue(double num);
			static v8::Handle<v8::Value> ToBooleanValue(bool b);
			static v8::Handle<v8::Value> DateToValue(double ms);

			static void PrintStack();
			static void PrintStack(int count);

			static const char* StateToString(GstState state);
			
			template <typename T>
			static void GstRefWrap(T* obj) {
				g_object_ref((GstObject*)obj);
			}

			template <typename T>
			static void GstUnrefWrap(T* obj) {
				g_object_unref((GstObject*)obj);
			}
		private:
			static v8::Local<v8::Value> CreateV8Error(const char* message);
	};
}

#endif