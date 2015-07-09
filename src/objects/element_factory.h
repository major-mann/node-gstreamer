#ifndef NSTR_EFACTORY_H
#define NSTR_EFACTORY_H

#include <node.h>
#include "plugin_feature_base.h"
#include "element.h"
#include "caps.h"

using namespace v8;

namespace nstr {

	class ElementFactory : public PluginFeatureBase<ElementFactory, GstElementFactory> {
		friend class LinkedObjectBase<ElementFactory, GstElementFactory>;
		public:
			/** Creates a new element factory using the specified name */
			static ElementFactory* Create(const char* factoryName);

			/** Creates a new element factory */
			static ElementFactory* Create(GstElementFactory* factory);

			/** Creates a new element factory from v8 arguments */
			static ElementFactory* Create(const FunctionCallbackInfo<Value>& args);

			/** Adds the constructor to the supplied instance */
			static void Init(Handle<Object> instance);

			/** Creates a new element with the given name */
			Element* CreateElement(const char* elementName);

			/** Returns the name of the gtype associated with the factory elements */
			Handle<Object> GetType();

			/** Returns the number of pad templates */
			uint GetPadTemplateCount();

			/** Gets whether pads or sinks are supported (or both, or none) */
			const char* SupportedUriType();

			//TODO: What does this do. Test
			//gst_element_factory_get_uri_protocols

			/** Checks whether the factory has the named interface. */
			bool HasInterface(const char* name);

			/** Checks if all supplied caps can be sinked */
			bool CanSinkAllCaps(Caps* caps);

			/** Checks if all supplied caps can be sourced */
			bool CanSrcAllCaps(Caps* caps);

			/** Checks if all supplied caps can be sinked */
			bool CanSrcAnyCaps(Caps* caps);

			/** Checks if all supplied caps can be sourced */
			bool CanSinkAnyCaps(Caps* caps);

			/** Returns an array of static pad templates */
			Handle<Array> StaticPadTemplates();

			/** Adds the required properties to the object */
			void InitializeV8Instance(Handle<Object> instance);

			//TODO
			//gst_element_factory_list_get_elements

			//TODO
			//gst_element_factory_list_is_type

		private:
			
			/**
			* Creates a new instance of the class.
			*/
			ElementFactory(GstElementFactory* factory);

			/** Returns the instance stored in the value */
			static ElementFactory* Instance(Handle<Value> data);

			/** The v8 Create wrapper */
			static void CreateElement(const FunctionCallbackInfo<Value>& args);

			/** Gets the type name managed by the factory */
			static void GetType(const FunctionCallbackInfo<Value>& args);

			/** Gets the number of pad templates */
			static void GetPadTemplateCount(const FunctionCallbackInfo<Value>& args);

			/** Gets the supported uris */
			static void SupportedUriType(const FunctionCallbackInfo<Value>& args);

			/** Returns whether the factory has the named interface */
			static void HasInterface(const FunctionCallbackInfo<Value>& args);

			/** Returns an array of pad templates */
			static void StaticPadTemplates(const FunctionCallbackInfo<Value>& args);

			/** Checks if all supplied caps can be sinked */
			static void CanSinkAllCaps(const FunctionCallbackInfo<Value>& args);

			/** Checks if all supplied caps can be sourced */
			static void CanSrcAllCaps(const FunctionCallbackInfo<Value>& args);

			/** Checks if all supplied caps can be sinked */
			static void CanSrcAnyCaps(const FunctionCallbackInfo<Value>& args);

			/** Checks if all supplied caps can be sourced */
			static void CanSinkAnyCaps(const FunctionCallbackInfo<Value>& args);
	};
}

#endif