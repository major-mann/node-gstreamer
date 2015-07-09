#include "g_object_wrap.h"
#include "../helper.h"
#include <string>
#include "element.h"

using namespace nstr;

std::vector<GType> GObjectWrap::SupportedTypes;

GObjectWrap* GObjectWrap::Create(GObject* obj) {
	if (SupportedTypes.size() == 0) {
		SupportedTypes.push_back(g_type_from_name("gchar"));
		SupportedTypes.push_back(g_type_from_name("gchararray"));
		SupportedTypes.push_back(g_type_from_name("gboolean"));
		SupportedTypes.push_back(g_type_from_name("gint"));
		SupportedTypes.push_back(g_type_from_name("guint"));
		SupportedTypes.push_back(g_type_from_name("gint8"));
		SupportedTypes.push_back(g_type_from_name("guint8"));
		SupportedTypes.push_back(g_type_from_name("gint16"));
		SupportedTypes.push_back(g_type_from_name("guint16"));
		SupportedTypes.push_back(g_type_from_name("gint32"));
		SupportedTypes.push_back(g_type_from_name("guint32"));
		SupportedTypes.push_back(g_type_from_name("gint64"));
		SupportedTypes.push_back(g_type_from_name("guint64"));
		SupportedTypes.push_back(g_type_from_name("gshort"));
		SupportedTypes.push_back(g_type_from_name("gushort"));
		SupportedTypes.push_back(g_type_from_name("glong"));
		SupportedTypes.push_back(g_type_from_name("gulong"));
		SupportedTypes.push_back(g_type_from_name("gfloat"));
		SupportedTypes.push_back(g_type_from_name("gdouble"));
		SupportedTypes.push_back(g_type_from_name("GstElement"));
		SupportedTypes.push_back(g_type_from_name("GstCaps"));
	}
	return LinkedObjectBase<GObjectWrap, GObject>::Create(obj);
}

GObjectWrap::GObjectWrap(GObject* gobj) 
	: LinkedObjectBase<GObjectWrap, GObject>(gobj, GObjectWrap::GObjectRefWrap, GObjectWrap::GObjectUnrefWrap) {
}

void GObjectWrap::InitializeV8Instance(Handle<Object> instance) {
	Local<External> data = External::New(Isolate::GetCurrent(), this);
	std::map<const char*, GType, cmp_str>::iterator it = Fields.begin();
	while (it != Fields.end()) {
		Helper::SetProp(instance, it->first, Undefined(Isolate::GetCurrent()));
		Fields.erase(it++);
	}

	//Get the properties contained on the object
	guint pcnt;
	GParamSpec** props = g_object_class_list_properties(G_OBJECT_GET_CLASS(GetObject()), &pcnt);
	GParamSpec** prop = props;
	//for (;prop - *props < pcnt; prop++) {
	for (uint i = 0; i < pcnt; i++) {
		if (!G_IS_PARAM_SPEC(*prop)) {
			//But sure why we get other data items in here?
			continue;
		}
		const char* name = g_param_spec_get_name(*prop);
		GType type = G_PARAM_SPEC_VALUE_TYPE(*prop);

		bool found = false;
		std::vector<GType>::iterator it = SupportedTypes.begin();
		while (it != SupportedTypes.end()) {
			GType st = *it;
			if (st == type) {
				Fields.insert(std::make_pair(name, type));
				Helper::SetAccessorProp(instance, name, PropertyGetAccessor, PropertySetAccessor, data);
				found = true;
				break;
			}
			it++;
		}

		if (!found) {
			printf("Unable to add field: \"%s\" with type \"%s\"\n", name, g_type_name(type));
		}
		g_param_spec_unref(*prop++);
	}
	delete props;
}

Handle<Value> GObjectWrap::GetValue(const char* fieldName) {
	std::map<const char*, GType>::iterator it = Fields.find(fieldName);
	if (it == Fields.end()) {
		return Handle<Value>();
	} else {
		GType type = it->second;
		const char* typeName = g_type_name(type);

		Isolate* iso = Isolate::GetCurrent();
		if (strcmp(typeName, "gchar") == 0) {
			char value;
			g_object_get(GetObject(), fieldName, &value, NULL);
			char* val = new char[1];
			val[0] = value;
			return String::NewFromUtf8(iso, val);
		} else if (strcmp(typeName, "gchararray") == 0) {
			gchar* value;
			g_object_get(GetObject(), fieldName, &value, NULL);
			if (value == NULL) {
				return Undefined(Isolate::GetCurrent());
			} else {
				return String::NewFromUtf8(iso, value);
			}
		} else if (strcmp(typeName, "gboolean") == 0) {
			gboolean value;
			g_object_get(GetObject(), fieldName, &value, NULL);
			return Integer::New(iso, value);
		} else if (strcmp(typeName, "gint") == 0) {
			gint value;
			g_object_get(GetObject(), fieldName, &value, NULL);
			return Integer::New(iso, value);
		} else if (strcmp(typeName, "guint") == 0) {
			guint value;
			g_object_get(GetObject(), fieldName, &value, NULL);
			return Integer::NewFromUnsigned(iso, value);
		} else if (strcmp(typeName, "gint8") == 0) {
			gint8 value;
			g_object_get(GetObject(), fieldName, &value, NULL);
			return Integer::New(iso, value);
		} else if (strcmp(typeName, "guint8") == 0) {
			guint8 value;
			g_object_get(GetObject(), fieldName, &value, NULL);
			return Integer::NewFromUnsigned(iso, value);
		} else if (strcmp(typeName, "gint16") == 0) {
			gint16 value;
			g_object_get(GetObject(), fieldName, &value, NULL);
			return Integer::New(iso, value);
		} else if (strcmp(typeName, "guint16") == 0) {
			guint16 value;
			g_object_get(GetObject(), fieldName, &value, NULL);
			return Integer::NewFromUnsigned(iso, value);
		} else if (strcmp(typeName, "gint32") == 0) {
			gint32 value;
			g_object_get(GetObject(), fieldName, &value, NULL);
			return Integer::New(iso, value);
		} else if (strcmp(typeName, "guint32") == 0) {
			guint32 value;
			g_object_get(GetObject(), fieldName, &value, NULL);
			return Integer::NewFromUnsigned(iso, value);
		} else if (strcmp(typeName, "gint64") == 0) {
			gint64 value;
			g_object_get(GetObject(), fieldName, &value, NULL);
			return Integer::New(iso, value);
		} else if (strcmp(typeName, "guint64") == 0) {
			guint64 value;
			g_object_get(GetObject(), fieldName, &value, NULL);
			return Integer::NewFromUnsigned(iso, value);
		} else if (strcmp(typeName, "gshort") == 0) {
			gshort value;
			g_object_get(GetObject(), fieldName, &value, NULL);
			return Integer::New(iso, value);
		} else if (strcmp(typeName, "gushort") == 0) {
			gushort value;
			g_object_get(GetObject(), fieldName, &value, NULL);
			return Integer::NewFromUnsigned(iso, value);
		} else if (strcmp(typeName, "glong") == 0) {
			glong value;
			g_object_get(GetObject(), fieldName, &value, NULL);
			return Integer::New(iso, value);
		} else if (strcmp(typeName, "gulong") == 0) {
			gulong value;
			g_object_get(GetObject(), fieldName, &value, NULL);
			return Integer::NewFromUnsigned(iso, value);
		} else if (strcmp(typeName, "gfloat") == 0) {
			gfloat value;
			g_object_get(GetObject(), fieldName, &value, NULL);
			return Number::New(iso, value);
		} else if (strcmp(typeName, "gdouble") == 0) {
			gdouble value;
			g_object_get(GetObject(), fieldName, &value, NULL);
			return Number::New(iso, value);
		} else if (strcmp(typeName, "GstElement") == 0) {
			GstElement* value = NULL;
			g_object_get(GetObject(), fieldName, &value, NULL);
			if (value == NULL) {
				return Null(Isolate::GetCurrent());
			} else {
				Element* ele = Element::Create(value);
				return ele->GetV8Instance();
			}
		} else if (strcmp(typeName, "GstCaps") == 0) {
			GstCaps* value = NULL;
			g_object_get(GetObject(), fieldName, &value, NULL);
			if (value == NULL) {
				return Null(Isolate::GetCurrent());
			} else {
				Caps* caps = Caps::Create(value);
				return caps->GetV8Instance();
			}
		} else {
			//Unsupported type!
			//TODO: Error?
			return Handle<Value>();
		}
	}
}
void GObjectWrap::SetValue(const char* fieldName, Handle<Value> value) {
	std::map<const char*, GType>::iterator it = Fields.find(fieldName);
	if (it == Fields.end()) {
		//Do nothing
		return;
	} else {
		GType type = it->second;
		const char* typeName = g_type_name(type);

		if (strcmp(typeName, "gchar") == 0) {
			if (value->IsString()) {
				String::Utf8Value uval(value->ToString());
				char c = (*uval)[0];
				g_object_set(GetObject(), fieldName, c, NULL);
			} else {
				char* err = new char[std::strlen(fieldName) + std::strlen("MUST be a string")];
				std::sprintf(err, "%s MUST be a string", fieldName);
				Helper::ThrowV8Error(err);
			}
		} else if (strcmp(typeName, "gchararray") == 0) {
			if (value->IsString()) {
				String::Utf8Value uval(value->ToString());
				g_object_set(GetObject(), fieldName, *uval, NULL);
			} else if (value->IsUndefined()) {
				g_object_set(GetObject(), fieldName, NULL, NULL);
			} else {
				char* err = new char[std::strlen(fieldName) + std::strlen("MUST be a string")];
				std::sprintf(err, "%s MUST be a string", fieldName);
				Helper::ThrowV8Error(err);
			}
		} else if (strcmp(typeName, "gboolean") == 0) {
			if (value->IsBoolean()) {
				bool val = value->IsTrue();
				g_object_set(GetObject(), fieldName, val, NULL);
			} else {
				char* err = new char[std::strlen(fieldName) + std::strlen("MUST be a boolean")];
				std::sprintf(err, "%s MUST be a boolean", fieldName);
				Helper::ThrowV8Error(err);
			}
		} else if (strcmp(typeName, "gint") == 0) {
			if (value->IsNumber()) {
				double val = value->NumberValue();
				int ival = static_cast<int>(val);
				g_object_set(GetObject(), fieldName, ival, NULL);
			} else {
				char* err = new char[std::strlen(fieldName) + std::strlen("MUST be a number")];
				std::sprintf(err, "%s MUST be a number", fieldName);
				Helper::ThrowV8Error(err);
			}
		} else if (strcmp(typeName, "guint") == 0) {
			if (value->IsNumber()) {
				double val = value->NumberValue();
				uint ival = static_cast<uint>(val);
				g_object_set(GetObject(), fieldName, ival, NULL);
			} else {
				char* err = new char[std::strlen(fieldName) + std::strlen("MUST be a number")];
				std::sprintf(err, "%s MUST be a number", fieldName);
				Helper::ThrowV8Error(err);
			}
		} else if (strcmp(typeName, "gint8") == 0) {
			if (value->IsNumber()) {
				double val = value->NumberValue();
				gint8 ival = static_cast<gint8>(val);
				g_object_set(GetObject(), fieldName, ival, NULL);
			} else {
				char* err = new char[std::strlen(fieldName) + std::strlen("MUST be a number")];
				std::sprintf(err, "%s MUST be a number", fieldName);
				Helper::ThrowV8Error(err);
			}
		} else if (strcmp(typeName, "guint8") == 0) {
			if (value->IsNumber()) {
				double val = value->NumberValue();
				guint ival = static_cast<guint>(val);
				g_object_set(GetObject(), fieldName, ival, NULL);
			} else {
				char* err = new char[std::strlen(fieldName) + std::strlen("MUST be a number")];
				std::sprintf(err, "%s MUST be a number", fieldName);
				Helper::ThrowV8Error(err);
			}
		} else if (strcmp(typeName, "gint16") == 0) {
			if (value->IsNumber()) {
				double val = value->NumberValue();
				gint16 ival = static_cast<gint16>(val);
				g_object_set(GetObject(), fieldName, ival, NULL);
			} else {
				char* err = new char[std::strlen(fieldName) + std::strlen("MUST be a number")];
				std::sprintf(err, "%s MUST be a number", fieldName);
				Helper::ThrowV8Error(err);
			}
		} else if (strcmp(typeName, "guint16") == 0) {
			if (value->IsNumber()) {
				double val = value->NumberValue();
				guint16 ival = static_cast<guint16>(val);
				g_object_set(GetObject(), fieldName, ival, NULL);
			} else {
				char* err = new char[std::strlen(fieldName) + std::strlen("MUST be a number")];
				std::sprintf(err, "%s MUST be a number", fieldName);
				Helper::ThrowV8Error(err);
			}
		} else if (strcmp(typeName, "gint32") == 0) {
			if (value->IsNumber()) {
				double val = value->NumberValue();
				gint32 ival = static_cast<gint32>(val);
				g_object_set(GetObject(), fieldName, ival, NULL);
			} else {
				char* err = new char[std::strlen(fieldName) + std::strlen("MUST be a number")];
				std::sprintf(err, "%s MUST be a number", fieldName);
				Helper::ThrowV8Error(err);
			}
		} else if (strcmp(typeName, "guint32") == 0) {
			if (value->IsNumber()) {
				double val = value->NumberValue();
				guint32 ival = static_cast<guint32>(val);
				g_object_set(GetObject(), fieldName, ival, NULL);
			} else {
				char* err = new char[std::strlen(fieldName) + std::strlen("MUST be a number")];
				std::sprintf(err, "%s MUST be a number", fieldName);
				Helper::ThrowV8Error(err);
			}
		} else if (strcmp(typeName, "gint64") == 0) {
			if (value->IsNumber()) {
				double val = value->NumberValue();
				gint64 ival = static_cast<gint64>(val);
				g_object_set(GetObject(), fieldName, ival, NULL);
			} else {
				char* err = new char[std::strlen(fieldName) + std::strlen("MUST be a number")];
				std::sprintf(err, "%s MUST be a number", fieldName);
				Helper::ThrowV8Error(err);
			}
		} else if (strcmp(typeName, "guint64") == 0) {
			if (value->IsNumber()) {
				double val = value->NumberValue();
				guint64 ival = static_cast<guint64>(val);
				g_object_set(GetObject(), fieldName, ival, NULL);
			} else {
				char* err = new char[std::strlen(fieldName) + std::strlen("MUST be a number")];
				std::sprintf(err, "%s MUST be a number", fieldName);
				Helper::ThrowV8Error(err);
			}
		} else if (strcmp(typeName, "gshort") == 0) {
			if (value->IsNumber()) {
				double val = value->NumberValue();
				gshort ival = static_cast<gshort>(val);
				g_object_set(GetObject(), fieldName, ival, NULL);
			} else {
				char* err = new char[std::strlen(fieldName) + std::strlen("MUST be a number")];
				std::sprintf(err, "%s MUST be a number", fieldName);
				Helper::ThrowV8Error(err);
			}
		} else if (strcmp(typeName, "gushort") == 0) {
			if (value->IsNumber()) {
				double val = value->NumberValue();
				gushort ival = static_cast<gushort>(val);
				g_object_set(GetObject(), fieldName, ival, NULL);
			} else {
				char* err = new char[std::strlen(fieldName) + std::strlen("MUST be a number")];
				std::sprintf(err, "%s MUST be a number", fieldName);
				Helper::ThrowV8Error(err);
			}
		} else if (strcmp(typeName, "glong") == 0) {
			if (value->IsNumber()) {
				double val = value->NumberValue();
				glong ival = static_cast<glong>(val);
				g_object_set(GetObject(), fieldName, ival, NULL);
			} else {
				char* err = new char[std::strlen(fieldName) + std::strlen("MUST be a number")];
				std::sprintf(err, "%s MUST be a number", fieldName);
				Helper::ThrowV8Error(err);
			}
		} else if (strcmp(typeName, "gulong") == 0) {
			if (value->IsNumber()) {
				double val = value->NumberValue();
				gulong ival = static_cast<gulong>(val);
				g_object_set(GetObject(), fieldName, ival, NULL);
			} else {
				char* err = new char[std::strlen(fieldName) + std::strlen("MUST be a number")];
				std::sprintf(err, "%s MUST be a number", fieldName);
				Helper::ThrowV8Error(err);
			}
		} else if (strcmp(typeName, "gfloat") == 0) {
			if (value->IsNumber()) {
				double val = value->NumberValue();
				gfloat ival = static_cast<gfloat>(val);
				g_object_set(GetObject(), fieldName, ival, NULL);
			} else {
				char* err = new char[std::strlen(fieldName) + std::strlen("MUST be a number")];
				std::sprintf(err, "%s MUST be a number", fieldName);
				Helper::ThrowV8Error(err);
			}
		} else if (strcmp(typeName, "gdouble") == 0) {
			if (value->IsNumber()) {
				double val = value->NumberValue();
				gdouble ival = static_cast<gdouble>(val);
				g_object_set(GetObject(), fieldName, ival, NULL);
			} else {
				char* err = new char[std::strlen(fieldName) + std::strlen("MUST be a number")];
				std::sprintf(err, "%s MUST be a number", fieldName);
				Helper::ThrowV8Error(err);
			}
		} else if (strcmp(typeName, "GstElement") == 0) {
			if (value->IsObject() || value->IsNull()) {
				if (value->IsNull()) {
					g_object_set(GetObject(), fieldName, NULL, NULL);
				} else {
					Element* ele = Element::Unwrap(value);
					g_object_set(GetObject(), fieldName, ele->GetObject(), NULL);
				}
			} else {
				char* err = new char[std::strlen(fieldName) + std::strlen("MUST be an Element")];
				std::sprintf(err, "%s MUST be an Element", fieldName);
				Helper::ThrowV8Error(err);
			}
		} else if (strcmp(typeName, "GstCaps") == 0) {
			if (value->IsObject() || value->IsNull()) {
				if (value->IsNull()) {
					g_object_set(GetObject(), fieldName, NULL, NULL);
				} else {
					Caps* caps = Caps::Unwrap(value);
					g_object_set(GetObject(), fieldName, caps->GetObject(), NULL);
				}
			} else {
				char* err = new char[std::strlen(fieldName) + std::strlen("MUST be a Caps")];
				std::sprintf(err, "%s MUST be a Caps", fieldName);
				Helper::ThrowV8Error(err);
			}
		}
	}
}

void GObjectWrap::GObjectRefWrap(GObject* obj) {
	g_object_ref(obj);
}
void GObjectWrap::GObjectUnrefWrap(GObject* obj) {
	g_object_unref(obj);
}

void GObjectWrap::PropertyGetAccessor(Local<String> prop, const PropertyCallbackInfo<Value>& info) {
	Local<External> data = Local<External>::Cast(info.Data());
	GObjectWrap* self = (GObjectWrap*)data->Value();
	String::Utf8Value fieldName(prop);
	Local<Value> value = self->GetValue(*fieldName);
	info.GetReturnValue().Set(value);
}

void GObjectWrap::PropertySetAccessor(Local<String> prop, Local<Value> value, const PropertyCallbackInfo<void>& info) {
	Local<External> data = Local<External>::Cast(info.Data());
	GObjectWrap* self = (GObjectWrap*)data->Value();
	String::Utf8Value fieldName(prop);
	self->SetValue(*fieldName, value);
}