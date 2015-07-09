#ifndef NSTR_STRUCTURE
#define NSTR_STRUCTURE

#include <node.h>
#include "linked_object_base.h"
#include <gst/gst.h>
#include <map>
#include <vector>

using namespace v8;
using namespace nstr;

class Structure : public LinkedObjectBase<Structure, GstStructure> {
	friend class LinkedObjectBase<Structure, GstStructure>;
	public:
		static Structure* Create(GstStructure* structure);

		/** Returns the name of the structure */
		const char* Name();

		/** Returns the total number of fields the structure contains */
		int FieldCount(); //gst_structure_n_fields

		/** Returns the name of the field at the given index */
		const char* FieldName(int index); //gst_structure_nth_field_name

		/** Returns the type of field at the given index */
		GType FieldType(const char* fieldName); //gst_structure_get_field_type
		
		/** Checks whether this structure's contents matches the supplied structure's contents */
		bool IsEqual(Structure* s);

		/** Checks whether this structure is a subset of the supplied structure */
		bool IsSubset(Structure* s);

		/** Checks whether this structure can intersect with the supplied structure leaving a non empty result */
		bool CanIntersect(Structure* s);

		/** Returns the intesect of the 2 structures */
		Structure* Intersect(Structure* s);

		/** Returns the structure as a string */
		const char* ToString();


		//gst_structure_nth_field_name

		//gst_structure_foreach - If all else fails we can get the keys from this

		//gst_structure_id_get
		/*
		For refcounted (mini)objects you will receive a new reference which you must release with a suitable _unref() when no longer needed. For strings and boxed types you will receive a copy which you will need to release with either g_free() or the suitable function for the boxed type.
		*/

		//http://stackoverflow.com/questions/18325623/v8objectsetaccessor-how-does-it-work
		//SetAccessor (Handle< String > name, AccessorGetter getter, AccessorSetter setter=0, Handle< Value > data=Handle< Value >(), AccessControl settings=DEFAULT, PropertyAttribute attribute=None)
	protected:
		void InitializeV8Instance(Handle<Object> instance);
	private:
		Structure(GstStructure* st);

		std::map<const char*, Structure*> V8WrapperData;
		Handle<Value> FieldValue(const char* name);

		static bool SupportedType(GType type);
		static std::vector<GQuark> SupportedTypes;
		static void InitializeSupportedTypes();

		static Structure* Instance(Local<Value> data);

		static void ValueTypeGetter(Local<String> property, const PropertyCallbackInfo<Value>& info);
		static void NameGetter(Local<String> property, const PropertyCallbackInfo<Value>& info);
		
		static void IsEqual(const FunctionCallbackInfo<Value>& args);
		static void IsSubset(const FunctionCallbackInfo<Value>& args);
		static void CanIntersect(const FunctionCallbackInfo<Value>& args);
		static void Intersect(const FunctionCallbackInfo<Value>& args);
		static void ToString(const FunctionCallbackInfo<Value>& args);
};

#endif
