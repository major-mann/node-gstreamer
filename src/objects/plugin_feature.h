#ifndef NSTR_PLUGIN_FEATURE_H
#define NSTR_PLUGIN_FEATURE_H

#include "linked_object_base.h"
#include "plugin_feature_base.h"
namespace nstr {

	class PluginFeature : public PluginFeatureBase<PluginFeature, GstPluginFeature> {
		friend class LinkedObjectBase<PluginFeature, GstPluginFeature>;
		
		public:
			static PluginFeature* Create(GstPluginFeature* plugin);

		private:
			PluginFeature(GstPluginFeature* plugin);
	};
}

#endif