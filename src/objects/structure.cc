#include "structure.h"
#include <algorithm>

std::vector<GQuark> Structure::SupportedTypes;

Structure::Structure(GstStructure* st) : LinkedObjectBase<Structure, GstStructure>(st, 0, gst_structure_free) { }

Structure* Structure::Create(GstStructure* structure) {
    return LinkedObjectBase<Structure, GstStructure>::Create(structure);
}

void Structure::InitializeV8Instance(Handle<Object> instance) {
    Isolate* isolate = Isolate::GetCurrent();

    if (SupportedTypes.size() == 0) {
        //Prepare the list of supported types.
        InitializeSupportedTypes();
    }

    //TODO: How to clear off old field acessors when re-initting? For now
    //  The structures should not be changing so we should be OK.

    int fcnt = FieldCount();
    for (int i = 0; i < fcnt; i++) {
        const char* fname = FieldName(i);
        GType ftype = FieldType(fname);

        //TODO: Should there be some kind of indication to the consumer
        //  that we have ignored a field
        if (!SupportedType(ftype)) {
            continue;
        }

        //Create a reference to the objecrt and field name
        std::pair<const char*, Structure*>* dataPair = new std::pair<const char*, Structure*>(fname, this);

        //We push this here so that cleanup will be taken care of for us with class destruction
        //TODO: Will this actuall work
        V8WrapperData.insert(*dataPair);
        Local<External> data = External::New(isolate, dataPair);
        Helper::SetAccessorProp(instance, fname, &ValueTypeGetter, data);
        
        //TODO: Note sure how to read and write dates? What GType are they?
        //gboolean  gst_structure_get_date ()
        //gboolean  gst_structure_get_date_time ()
        //gboolean  gst_structure_get_clock_time ()

        //TODO: Implement once time for experimentation is available
        //gboolean  gst_structure_get_enum ()
        //gboolean  gst_structure_get_fraction ()
    }

    //TODO: Add functions here
}

Handle<Value> Structure::FieldValue(const char* name) {
    GType type = gst_structure_get_field_type(GetObject(), name);
    if (type == g_type_from_name("gboolean")) {
        gboolean result;
        gst_structure_get_boolean(GetObject(), name, &result);
        return Helper::ToBooleanValue(result);
    } else if (type == g_type_from_name("gint")) {
        gint result;
        gst_structure_get_int(GetObject(), name, &result);
        return Helper::ToValue(result);
    } else if (type == g_type_from_name("guint")) {
        guint result;
        gst_structure_get_uint(GetObject(), name, &result);
        return Helper::ToValue(result);
    } else if (type == g_type_from_name("gdouble")) {
        gdouble result;
        gst_structure_get_double(GetObject(), name, &result);
        return Helper::ToValue(result);
    } else if (type == g_type_from_name("gchar")) {
        const char* result = gst_structure_get_string(GetObject(), name);
        return Helper::ToValue(result);
    } else {
        return v8::Undefined(Isolate::GetCurrent());
    }
}

bool Structure::SupportedType(GType type) {
    GQuark qname = g_type_qname(type);
    return std::binary_search (SupportedTypes.begin(), 
        SupportedTypes.end(), 
        qname);
}

void Structure::InitializeSupportedTypes() {
    SupportedTypes.push_back(g_type_qname(g_type_from_name("gboolean")));
    SupportedTypes.push_back(g_type_qname(g_type_from_name("gint")));
    SupportedTypes.push_back(g_type_qname(g_type_from_name("guint")));
    SupportedTypes.push_back(g_type_qname(g_type_from_name("gdouble")));
    SupportedTypes.push_back(g_type_qname(g_type_from_name("gchar")));
    std::sort (SupportedTypes.begin(), SupportedTypes.end());
}

/** Returns the name of the structure */
const char* Structure::Name() {
    const char* name = gst_structure_get_name(GetObject());
    return name;
}

/** Returns the total number of fields the structure contains */
int Structure::FieldCount() {
    return gst_structure_n_fields(GetObject());
} //gst_structure_n_fields

/** Returns the name of the field at the given index */
const char* Structure::FieldName(int index) {
    return gst_structure_nth_field_name(GetObject(), index);
} //gst_structure_nth_field_name

/** Returns the type of field at the given index */
GType Structure::FieldType(const char* fieldName) {
    return gst_structure_get_field_type(GetObject(), fieldName);
} //gst_structure_get_field_type

/** Checks whether this structure's contents matches the supplied structure's contents */
bool Structure::IsEqual(Structure* s) {
    return gst_structure_is_equal(GetObject(), s->GetObject());
}

/** Checks whether this structure is a subset of the supplied structure */
bool Structure::IsSubset(Structure* s) {
    return gst_structure_is_subset(GetObject(), s->GetObject());
}

/** Checks whether this structure can intersect with the supplied structure leaving a non empty result */
bool Structure::CanIntersect(Structure* s) {
    return gst_structure_can_intersect(GetObject(), s->GetObject());
}

/** Returns the intesect of the 2 structures */
Structure* Structure::Intersect(Structure* s) {
    GstStructure* result = gst_structure_intersect(GetObject(), s->GetObject());
    return new Structure(result);
}

/** Returns the structure as a string */
const char* Structure::ToString() {
    return gst_structure_to_string(GetObject());
}

Structure* Structure::Instance(Local<Value> data) {
    Local<External> ext = Local<External>::Cast(data);
    return (Structure*)ext->Value();
}

void Structure::ValueTypeGetter(Local<String> property, const PropertyCallbackInfo<Value>& info) {
    Local<External> ext = Local<External>::Cast(info.Data());
    std::pair<const char*, Structure*>* pair = (std::pair<const char*, Structure*>*)ext->Value();
    Local<Value> value = pair->second->FieldValue(pair->first);
    info.GetReturnValue().Set(value);
}
void Structure::NameGetter(Local<String> property, const PropertyCallbackInfo<Value>& info) {
    Structure* s = Instance(info.Data());
    info.GetReturnValue().Set(Helper::ToValue(s->Name()));
}
void Structure::IsEqual(const FunctionCallbackInfo<Value>& args) {
    Structure* s1 = Instance(args.Data());
    Local<Object> obj = args[0]->ToObject();
    Structure* s2 = node::ObjectWrap::Unwrap<Structure>(obj);
    args.GetReturnValue().Set(Helper::ToValue(s1->IsEqual(s2)));
}
void Structure::IsSubset(const FunctionCallbackInfo<Value>& args) {
    Structure* s1 = Instance(args.Data());
    Local<Object> obj = args[0]->ToObject();
    Structure* s2 = node::ObjectWrap::Unwrap<Structure>(obj);
    args.GetReturnValue().Set(Helper::ToValue(s1->IsSubset(s2)));
}
void Structure::CanIntersect(const FunctionCallbackInfo<Value>& args) {
    Structure* s1 = Instance(args.Data());
    Local<Object> obj = args[0]->ToObject();
    Structure* s2 = node::ObjectWrap::Unwrap<Structure>(obj);
    args.GetReturnValue().Set(Helper::ToValue(s1->CanIntersect(s2)));
}
void Structure::Intersect(const FunctionCallbackInfo<Value>& args)  {
    Structure* s1 = Instance(args.Data());
    Local<Object> obj = args[0]->ToObject();
    Structure* s2 = node::ObjectWrap::Unwrap<Structure>(obj);
    Structure* inter = s1->Intersect(s2);
    args.GetReturnValue().Set(inter->GetV8Instance());
}
void Structure::ToString(const FunctionCallbackInfo<Value>& args) {
    Structure* instance = Instance(args.Data());
    args.GetReturnValue().Set(Helper::ToValue(instance->ToString()));
}


