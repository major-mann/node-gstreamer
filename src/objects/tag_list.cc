#include "tag_list.h"

using namespace nstr;

std::vector<GType> TagList::SupportedTypes;

TagList::TagList(GstTagList* list) : LinkedObjectBase<TagList, GstTagList>(list, TagList::RefWrap, TagList::UnrefWrap) {

}

TagList* TagList::Create(GstTagList* list) {
	if (SupportedTypes.size() == 0) {
		SupportedTypes.push_back(g_type_from_name("gchararray"));
		SupportedTypes.push_back(g_type_from_name("gboolean"));
		SupportedTypes.push_back(g_type_from_name("gint"));
		SupportedTypes.push_back(g_type_from_name("guint"));
		SupportedTypes.push_back(g_type_from_name("gint64"));
		SupportedTypes.push_back(g_type_from_name("guint64"));
		SupportedTypes.push_back(g_type_from_name("gfloat"));
		SupportedTypes.push_back(g_type_from_name("gdouble"));
	}
	return LinkedObjectBase<TagList, GstTagList>::Create(list);
}

const char* TagList::GetScope() {
	GstTagScope scope =  gst_tag_list_get_scope(GetObject());

	switch (scope) {
		case GST_TAG_SCOPE_STREAM:
			return "stream";
		case GST_TAG_SCOPE_GLOBAL:
			return "global";
		default:
			printf("%s\n", "Critical error. Unexpected GstTagScope received!");
			exit(-1);
			return NULL;
	}
}

const char* TagList::ToString() {
	return gst_tag_list_to_string(GetObject());
}

void TagList::RefWrap(GstTagList* list) {
	gst_tag_list_ref(list);
}
void TagList::UnrefWrap(GstTagList* list) {
	gst_tag_list_unref(list);
}

bool TagList::GetWritable() {
	return gst_tag_list_is_writable(GetObject());
}

Handle<Array> TagList::Names() {
	Local<Array> result = Array::New(Isolate::GetCurrent());
	int size = gst_tag_list_n_tags(GetObject());
	for (int i = 0; i < size; i++) {
		result->Set(i, Helper::ToValue(gst_tag_list_nth_tag_name(GetObject(), i)));
	}
	return result;
}

Handle<Array> TagList::Tags() {
	Local<Array> result = Array::New(Isolate::GetCurrent());
	int size = gst_tag_list_n_tags(GetObject());
	for (int i = 0; i < size; i++) {
		result->Set(i, Tag(gst_tag_list_nth_tag_name(GetObject(), i)));
	}
	return result;
}

bool TagList::Exists(const char* name) {
	return gst_tag_exists(name);
}

Handle<Object> TagList::Tag(const char* name) {

	Local<Object> tag = Object::New(Isolate::GetCurrent());

	std::map<const char*, TagList*, cmp_str>::iterator it = StoredTags.find(name);
	std::pair<const char*, TagList*>* pair = NULL;
	if (it == StoredTags.end()) {
		//Add the reference
		StoredTags.insert(std::make_pair(name, this));
		it = StoredTags.find(name);
	}
	std::pair<const char*, TagList*> tmp = *it;
	pair = &tmp;

	Local<External> data = External::New(Isolate::GetCurrent(), &pair);
	Helper::SetAccessorProp(tag, "id", TagIdGetter, data);
	Helper::SetAccessorProp(tag, "nick", TagNickGetter, data);
	Helper::SetAccessorProp(tag, "description", TagDescriptionGetter, data);
	Helper::SetAccessorProp(tag, "flag", TagFlagGetter, data);
	Helper::SetAccessorProp(tag, "value", TagValueGetter, data);

	return tag;
}

const char* TagList::TagNick(const char* name) {
	return gst_tag_get_nick(name);
}
const char* TagList::TagDescription(const char* name) {
	return gst_tag_get_description(name);
}
const char* TagList::TagFlag(const char* name) {
	GstTagFlag flag = gst_tag_get_flag(name);

	switch (flag) {
		case GST_TAG_FLAG_UNDEFINED:
			return "undefined";
		case GST_TAG_FLAG_META:
			return "meta";
		case GST_TAG_FLAG_ENCODED:
			return "encoded";
		case GST_TAG_FLAG_DECODED:
			return "decoded";
		case GST_TAG_FLAG_COUNT:
			return "count";
		default:
			printf("%s\n", "Critical error! Unrecognized flag value!");
			exit(-1);
			return NULL;
	}
}

Handle<Value> TagList::TagValue(const char* tag) {
	if (gst_tag_is_fixed(tag)) {
		GType type = gst_tag_get_type(tag);
		const char* typeName = g_type_name(type);

		Isolate* iso = Isolate::GetCurrent();
		if (strcmp(typeName, "gchararray") == 0) {
			gchar* value;
			gst_tag_list_get_string(GetObject(), tag, &value);
			if (value == NULL) {
				return Undefined(Isolate::GetCurrent());
			} else {
				return String::NewFromUtf8(iso, value);
			}
		} else if (strcmp(typeName, "gboolean") == 0) {
			gboolean value;
			gst_tag_list_get_boolean(GetObject(), tag, &value);
			return Integer::New(iso, value);
		} else if (strcmp(typeName, "gint") == 0) {
			gint value;
			gst_tag_list_get_int(GetObject(), tag, &value);
			return Integer::New(iso, value);
		} else if (strcmp(typeName, "guint") == 0) {
			guint value;
			gst_tag_list_get_uint(GetObject(), tag, &value);
			return Integer::NewFromUnsigned(iso, value);
		} else if (strcmp(typeName, "gint64") == 0) {
			gint64 value;
			gst_tag_list_get_int64(GetObject(), tag, &value);
			return Integer::New(iso, value);
		} else if (strcmp(typeName, "guint64") == 0) {
			guint64 value;
			gst_tag_list_get_uint64(GetObject(), tag, &value);
			return Integer::NewFromUnsigned(iso, value);
		} else if (strcmp(typeName, "gfloat") == 0) {
			gfloat value;
			gst_tag_list_get_float(GetObject(), tag, &value);
			return Number::New(iso, value);
		} else if (strcmp(typeName, "gdouble") == 0) {
			gdouble value;
			gst_tag_list_get_double(GetObject(), tag, &value);
			return Number::New(iso, value);
		} else {
			//Unsupported type!
			//TODO: Error?
			return Handle<Value>();
		}
	} else {
		Local<Array> arr = Array::New(Isolate::GetCurrent());
		uint size = gst_tag_list_get_tag_size(GetObject(), tag);
		for (uint i = 0; i < size; i++) {
			arr->Set(i, TagValue(tag, i));
		}
		return arr;
	}
}
Handle<Value> TagList::TagValue(const char* tag, uint index) {
	GType type = gst_tag_get_type(tag);
	const char* typeName = g_type_name(type);

	Isolate* iso = Isolate::GetCurrent();
	if (strcmp(typeName, "gchararray") == 0) {
		gchar* value;
		gst_tag_list_get_string_index(GetObject(), tag, index, &value);
		if (value == NULL) {
			return Undefined(Isolate::GetCurrent());
		} else {
			return String::NewFromUtf8(iso, value);
		}
	} else if (strcmp(typeName, "gboolean") == 0) {
		gboolean value;
		gst_tag_list_get_boolean_index(GetObject(), tag, index, &value);
		return Integer::New(iso, value);
	} else if (strcmp(typeName, "gint") == 0) {
		gint value;
		gst_tag_list_get_int_index(GetObject(), tag, index, &value);
		return Integer::New(iso, value);
	} else if (strcmp(typeName, "guint") == 0) {
		guint value;
		gst_tag_list_get_uint_index(GetObject(), tag, index, &value);
		return Integer::NewFromUnsigned(iso, value);
	} else if (strcmp(typeName, "gint64") == 0) {
		gint64 value;
		gst_tag_list_get_int64_index(GetObject(), tag, index, &value);
		return Integer::New(iso, value);
	} else if (strcmp(typeName, "guint64") == 0) {
		guint64 value;
		gst_tag_list_get_uint64_index(GetObject(), tag, index, &value);
		return Integer::NewFromUnsigned(iso, value);
	} else if (strcmp(typeName, "gfloat") == 0) {
		gfloat value;
		gst_tag_list_get_float_index(GetObject(), tag, index, &value);
		return Number::New(iso, value);
	} else if (strcmp(typeName, "gdouble") == 0) {
		gdouble value;
		gst_tag_list_get_double_index(GetObject(), tag, index, &value);
		return Number::New(iso, value);
	} else {
		//Unsupported type!
		//TODO: Error?
		return Handle<Value>();
	}
}

void TagList::InitializeV8Instance(Handle<Object> instance) {
	Local<External> data = External::New(Isolate::GetCurrent(), this);

	Helper::SetAccessorProp(instance, "scope", GetScopeGetter, data);

	Helper::SetFunctionProp(instance, "toString", ToString, data);
	Helper::SetFunctionProp(instance, "names", ToString, data);
	Helper::SetFunctionProp(instance, "tags", ToString, data);
	Helper::SetFunctionProp(instance, "exists", ToString, data);
	Helper::SetFunctionProp(instance, "tag", ToString, data);
}

TagList* TagList::Instance(Local<Value> data) {
	Local<External> ext = Local<External>::Cast(data);
	return (TagList*)ext->Value();
}

void TagList::GetScopeGetter(Local<String> prop, const PropertyCallbackInfo<Value>& info) {
	TagList* self =  Instance(info.Data());
	info.GetReturnValue().Set(Helper::ToValue(self->GetScope()));
}

void TagList::ToString(const FunctionCallbackInfo<Value>& args) {
	TagList* self =  Instance(args.Data());
	args.GetReturnValue().Set(Helper::ToValue(self->ToString()));
}
void TagList::Names(const FunctionCallbackInfo<Value>& args) {
	TagList* self =  Instance(args.Data());
	args.GetReturnValue().Set(self->Names());
}
void TagList::Tags(const FunctionCallbackInfo<Value>& args) {
	TagList* self =  Instance(args.Data());
	args.GetReturnValue().Set(self->Tags());
}
void TagList::Exists(const FunctionCallbackInfo<Value>& args) {
	if (args.Length() > 0 && args[0]->IsString()) {
		TagList* self =  Instance(args.Data());
		String::Utf8Value uval(args[0]->ToString());
		args.GetReturnValue().Set(Helper::ToBooleanValue(self->Exists(*uval)));
	} else {
		Helper::ThrowV8Error("exists expects a string name parameter to be supplied");
	}
}
void TagList::Tag(const FunctionCallbackInfo<Value>& args) {
	if (args.Length() > 0 && args[0]->IsString()) {
		TagList* self =  Instance(args.Data());
		String::Utf8Value uval(args[0]->ToString());
		args.GetReturnValue().Set(self->Tag(*uval));
	} else {
		Helper::ThrowV8Error("tag expects a string name parameter to be supplied");
	}	
}

void TagList::TagIdGetter(Local<String> prop, const PropertyCallbackInfo<Value>& info) {
	std::pair<const char*, TagList*>* data = (std::pair<const char*, TagList*>*)(Local<External>::Cast(info.Data())->Value());
	const char* id = data->first;
	info.GetReturnValue().Set(Helper::ToValue(id));
}
void TagList::TagNickGetter(Local<String> prop, const PropertyCallbackInfo<Value>& info) {
	std::pair<const char*, TagList*>* data = (std::pair<const char*, TagList*>*)(Local<External>::Cast(info.Data())->Value());
	const char* nick = data->second->TagNick(data->first);
	info.GetReturnValue().Set(Helper::ToValue(nick));
}
void TagList::TagDescriptionGetter(Local<String> prop, const PropertyCallbackInfo<Value>& info) {
	std::pair<const char*, TagList*>* data = (std::pair<const char*, TagList*>*)(Local<External>::Cast(info.Data())->Value());
	const char* description = data->second->TagDescription(data->first);
	info.GetReturnValue().Set(Helper::ToValue(description));
}
void TagList::TagFlagGetter(Local<String> prop, const PropertyCallbackInfo<Value>& info) {
	std::pair<const char*, TagList*>* data = (std::pair<const char*, TagList*>*)(Local<External>::Cast(info.Data())->Value());
	const char* flag = data->second->TagFlag(data->first);
	info.GetReturnValue().Set(Helper::ToValue(flag));
}
void TagList::TagValueGetter(Local<String> prop, const PropertyCallbackInfo<Value>& info) {
	std::pair<const char*, TagList*>* data = (std::pair<const char*, TagList*>*)(Local<External>::Cast(info.Data())->Value());
	const char* flag = data->second->TagFlag(data->first);
	info.GetReturnValue().Set(Helper::ToValue(flag));
}