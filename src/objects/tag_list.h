#ifndef NSTR_TAG_LIST_H
#define NSTR_TAG_LIST_H

#include <node.h>
#include <gst/gst.h>
#include "linked_object_base.h"
#include "../helper.h"

namespace nstr {

	class TagList : public LinkedObjectBase<TagList, GstTagList> {
		friend class LinkedObjectBase<TagList, GstTagList>;

		public:
			static TagList* Create(GstTagList* list);

			const char* GetScope();
			const char* ToString();
			bool GetWritable();
			TagList* Copy();
			Handle<Array> Names();
			Handle<Array> Tags();
			bool Exists(const char* name);
			Handle<Object> Tag(const char* name);

			//Tag methods
			const char* TagNick(const char* name);
			const char* TagDescription(const char* name);
			const char* TagFlag(const char* name);
			Handle<Value> TagValue(const char* tag);
			Handle<Value> TagValue(const char* tag, uint index);

		protected:
			TagList(GstTagList* list);
			void InitializeV8Instance(Handle<Object> instance);

		private:
			std::map<const char*, TagList*, cmp_str> StoredTags;
			static std::vector<GType> SupportedTypes;
			static void RefWrap(GstTagList* list);
			static void UnrefWrap(GstTagList* list);

			//Tag list members
			static TagList* Instance(Local<Value> data);
			static void GetScopeGetter(Local<String> prop, const PropertyCallbackInfo<Value>& info);
			static void ToString(const FunctionCallbackInfo<Value>& args);
			static void Names(const FunctionCallbackInfo<Value>& args);
			static void Tags(const FunctionCallbackInfo<Value>& args);
			static void Exists(const FunctionCallbackInfo<Value>& args);
			static void Tag(const FunctionCallbackInfo<Value>& args);

			//Tag members
			static void TagIdGetter(Local<String> prop, const PropertyCallbackInfo<Value>& info);
			static void TagNickGetter(Local<String> prop, const PropertyCallbackInfo<Value>& info);
			static void TagDescriptionGetter(Local<String> prop, const PropertyCallbackInfo<Value>& info);
			static void TagFlagGetter(Local<String> prop, const PropertyCallbackInfo<Value>& info);
			static void TagValueGetter(Local<String> prop, const PropertyCallbackInfo<Value>& info);
			//See g_object_wrap for conversion...
	};

}

#endif
