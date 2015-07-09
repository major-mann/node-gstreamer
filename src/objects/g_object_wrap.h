#ifndef NSTR_GOBJECT_WRAP
#define NSTR_GOBJECT_WRAP

#include <node.h>
#include <gst/gst.h> //TODO: Perhaps we have a more appropriate header?
#include "../helper.h"
#include "linked_object_base.h"
#include <map>
#include <vector>

using namespace v8;
namespace nstr {
	class GObjectWrap : public LinkedObjectBase<GObjectWrap, GObject> {
		friend class LinkedObjectBase<GObjectWrap, GObject>;
		public:
			static GObjectWrap* Create(GObject* obj);
		protected:
			GObjectWrap(GObject* obj);
			virtual Handle<Value> GetValue(const char* fieldName);
			void SetValue(const char* fieldName, Handle<Value>);
			void InitializeV8Instance(Handle<Object> instance);
		private:
			std::map<const char*, GType, cmp_str> Fields;
			static std::vector<GType> SupportedTypes;
			static void PropertyGetAccessor(Local<String> prop, const PropertyCallbackInfo<Value>& info);
			static void PropertySetAccessor(Local<String> prop, Local<Value> value, const PropertyCallbackInfo<void>& info);
			static void GObjectRefWrap(GObject* obj);
			static void GObjectUnrefWrap(GObject* obj);
		
	};
}

#endif