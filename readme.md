# Node GStreamer
This package seeks to provide feature parity with the GStreamer API inside of node.js, with only the most mintor adjustments to attempt to keep usage very similar in Javascript to usage in C while migrating accross to a javascript style.

## Project status
This is a very early release with only basic functionality (Mostly write functionality). It has been built on OSX, manually linking to the GStreamer libraries, and only tested there. It probably leaks like a sieve, and has some rather major design issues which need to be solved. Also, as the developer of this project I had no previous experience with GStreamer (other than reading over the basic concepts of the project), and very limited C++ experience (I have written a proprietary node.js wrapper for libav.), so the code style and patterns may seem strange to those experienced with these technologies. Any feedback on this is always appreciated.

## Getting started
* Make sure you have a sane build environment (on OSX xcode command line tools)
* Installed the gstreamer 1.* sdk (expected to be at /Library/Frameworks/GStreamer.framework)
* git clone this repo
* npm install (This should run node-gyp rebuild)
* npm test (You should here the intro to ACDC Back in Black start playing)

##Design issues
### Base class structure
Currently the base class structure is using templates in a rather fishy manner (it has self referencing inheritance templates), and because of the template structure does not handle polymorphism very well.

This needs to be changed to exclude the self referencing, and we need to follow the GObject hierarchy to ensure we are able to take advantage of polymorphism.

### Deprecated checks
The gobject field adders should check for the deprecated flags.

### GValue type conversion
We need to generalised system of type conversion that can be used from all parts of the application. This should include the ability to register new type converters to be used when encountering non value types that our application understands.
There may even be an opportunity to wrap the GObject structure in such a way that we can expose types which we do not understand into the Javascript world automatically (Although as a developer I need to understand C++ function pointers better before I can achieve this I think)

### V8 instance initialization
Right now this is a bit of a mess, and has incomplete parts, it needs to be cleaned up a little (although this can wait till after the structure changes to choose the most appropriate way to go about this)

