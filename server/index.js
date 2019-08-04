"use strict";

// Dependencies
const Log = require("../lib/require/log.js")("server");
const Exception = require("../lib/require/exception.js")("server");
const Web = require("../lib/require/web.js");
const FileSystem = require("../lib/require/filesystem.js");

// Standard library
const Path = require("path");
const TimeSeries = require("../lib/require/persistence/timeseries.js");

(async () => {

	const pathDist = Path.resolve(__dirname, "..", ".dist");
	const pathData = Path.resolve(__dirname, "..", "data");

	// Create directory if it does not exists
	await FileSystem.mkdir(pathDist);
	await FileSystem.mkdir(pathData);

	let timeseries = new TimeSeries(Path.join(pathData, "timeseries"));
	await timeseries.waitReady();
	Log.info("Timeseries is ready");

	timeseries.insert(12, "A");
	timeseries.insert(14, "B");
	timeseries.insert(11, "C");

	timeseries.forEach((timestamp, data) => {
		console.log(timestamp, data);
	}, 0);

	// Set-up the web server
	let web = new Web(8001, {
		rootDir: pathDist,
	});

	// Handle the normal page request
	web.addRoute("get", "/api/", async (request, response) => {
		response.sendStatus(404);
	});

	// Start the web server
	web.start();
})();
