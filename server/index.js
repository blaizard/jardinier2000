"use strict";

// Local
const API = require("./api.js");

// Dependencies
const Log = require("../lib/require/log.js")("server");
const Exception = require("../lib/require/exception.js")("server");
const Web = require("../lib/require/web.js");
const FileSystem = require("../lib/require/filesystem.js");
const TimeSeries = require("../lib/require/persistence/timeseries.js");
const TaskManager = require("../lib/require/task/manager-singleton.js");

// Standard library
const Path = require("path");

(async () => {

	const pathDist = Path.resolve(__dirname, "..", ".dist");
	const pathData = Path.resolve(__dirname, "..", "data");

	// Create directory if it does not exists
	await FileSystem.mkdir(pathDist);
	await FileSystem.mkdir(pathData);

	let timeseries = new TimeSeries(Path.join(pathData, "timeseries"), {
		unique: true,
		uniqueMerge: (a, b) => {
			return Object.assign(a, b);
		}
	});
	await timeseries.waitReady();
	Log.info("Persisted time series module is ready");

	/*
	await timeseries.insert(12, "A");
	await timeseries.insert(14, "B");
	await timeseries.insert(11, "C");
	await timeseries.insert(12, "A");
	await timeseries.insert(14, "B");

	timeseries.forEach((timestamp, data) => {
		console.log(timestamp, data);
	});
	*/

	// Set-up the web server
	let web = new Web(8001, {
		rootDir: pathDist,
	});

	// Deploy APIs
	let api = new API(web, {
		timeseries: timeseries
	});
	api.deploy();

	// Start the web server
	web.start();
})();
