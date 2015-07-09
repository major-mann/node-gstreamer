(function ogg_player(addon) {

	var filename = process.argv[2];
	if (filename) {
		main(filename);
	} else {
		console.log('usage node ogg.js filename');
	}

	function main(filename) {
		console.log('playing file "%s"', filename);

		var pipeline = new addon.Pipeline("audio-player"),
			source = new addon.Element("filesrc", "file-source"),
			demuxer = new addon.Element("oggdemux", "ogg-demuxer"),
			decoder = new addon.Element("vorbisdec", "vorbis-decoder"),
			conv = new addon.Element("audioconvert", "converter"),
			sink = new addon.Element("autoaudiosink", "audio-output"),
			bus = pipeline.bus();

		//Set where to retrieve the file from.
		source.settings.location = filename;

		//TODO: Not sure why, but the position
		//	and duration seem to be updated every 
		//	1 second only. Need to find a way to 
		//	increase update rate
		var duration;
		var inter = setInterval(function() {
			//var position = source.position("percent");
			var position = pipeline.position('time');
			position = Math.round(position / duration * 100000) / 1000;
			position = position + '%';
			console.log(position);
			//console.log('%s of %s', position, duration);
		}, 200);

		bus.watch();
		bus.on('watch', function(msg, arg1, arg2) {
			//console.log('watch event "%s", %s, %s', msg, arg1, arg2);

			switch (msg) {
				case 'eos':
					clearInterval(inter);
					addon.mainLoop.stop();
					break;
				case 'new_clock':
					console.log("0%");
					duration = pipeline.duration('time');
					break;
				default:
					break;
			}
		});

		//TODO: We may need to add some events to pipeline....

		//TODO: Test what happens when we add invalid objects (ie. non-elements)
		pipeline.add(source);
		pipeline.add(demuxer);
		pipeline.add(decoder);
		pipeline.add(conv);
		pipeline.add(sink);

		//Link up all elements we can
		source.link(demuxer);
		//Note: We will link demuxer to decoder in the decoder pad added event
		decoder.link(conv);
		conv.link(sink);

		demuxer.on('pad-added', function demuxerPadAdded(pad) {
			//Get the sink pad
			var sink = decoder.staticPad('sink');

			//Link demuxer and decoder
			pad.link(sink);

			//console.log('Finished connecting pads');
		});

		//TODO: Why is this playing without calling run?
		pipeline.setState('playing', function(state) {
			addon.mainLoop.run(function() {
				console.log('Run complete!');
			});
		});

	}

}(require('./build/Release/addon')));