"use strict";

// Dependencies
const Log = require("../lib/require/log.js")("server");
const Exception = require("../lib/require/exception.js")("server");
const Web = require("../lib/require/web.js");
const FileSystem = require("../lib/require/filesystem.js");

// Standard library
const Path = require("path");

(async () => {
	const pathDist = Path.resolve(__dirname, "..", ".dist");

	// Create directory if it does not exists
	await FileSystem.mkdir(pathDist);

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
