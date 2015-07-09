(function node_gstreamer(exports, addon) {

	//Expose global functions
	exports.version = addon.version;

	//Singletons
	exports.registry = addon.registry;
	exports.mainLoop = addon.mainLoop;

	//Types
	exports.Element = addon.Element;
	exports.ElementFactory = addon.ElementFactory;
	exports.Pipeline = addon.Pipeline;

}(module.exports, require('./build/Release/addon')));
