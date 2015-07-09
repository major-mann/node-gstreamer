#include <gst/gst.h>
#include "plugin_feature.h"
#include "plugin.h"
#include "../helper.h"

PluginFeature* PluginFeature::Create(GstPluginFeature* pf) {
	return LinkedObjectBase<PluginFeature, GstPluginFeature>::Create(pf);
}

PluginFeature::PluginFeature(GstPluginFeature* plugin) : PluginFeatureBase(plugin) { }