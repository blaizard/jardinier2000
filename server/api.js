"use strict";

const Log = require("../lib/require/log.js")("api");
const Exception = require("../lib/require/exception.js")("api");
const Web = require("../lib/require/web.js");

const Config = require("../config.json");

module.exports = class API {
	constructor(web, options = {}) {
		this.web = web;
		this.options = options;
	}

	deploy() {
		Log.info("Deploying APIs");
		this.deploySample();
	}

	/**
	 * Return the current timestamp with second precision
	 */
	getCurrentTimestampS() {
		return Math.floor(Date.now() / 1000);
	}

	deploySample() {
		Exception.assert(this.options.hasOwnProperty("timeseries"), "Missing timeseries option for sample API");

		this.web.addRoute("post", "/api/v1/sample", async (request, response) => {

			// Set the options
			const options = Object.assign({
				token: undefined
			}, request.query);
			Exception.assert(typeof options.token !== "undefined", "The query 'token' must be set.");
			Exception.assert(Config.nodes.tokens.hasOwnProperty(options.token), "The token '" + options.token + "' is not registered.");
			Exception.assert(request.body instanceof Object, "The body should contain a json formated data, instead received: " + JSON.stringify(request.body));
			Exception.assert(request.body.hasOwnProperty("list"), "The body should contain a list entry, instead received: " + JSON.stringify(request.body));
			Exception.assert(request.body.list instanceof Array, "The list should contain a json formated Array, instead received: " + JSON.stringify(request.body));

			const NodeId = Config.nodes.tokens[options.token];
			const timestamp = this.getCurrentTimestampS();
			const validKeys = {
				timestamp: "number",
				temperature: { "*": "number" },
				humidity: { "*": "number" },
				luminosity: { "*": "number" },
				moisture: { "*": "number" },
				battery: { "*": "number" },
			}

			const validate = (object, key, rule) => {
				if (!rule.hasOwnProperty("*")) {
					Exception.assert(rule.hasOwnProperty(key), () => ("The key '" + key + "' is invalid in " + JSON.stringify(object)));
				}

				const subRule = (rule.hasOwnProperty(key)) ? rule[key] : rule["*"];
				if (typeof subRule === "string") {
					Exception.assert(typeof object[key] === subRule, () => ("The key '" + key + "' is of wrong type in "+ JSON.stringify(object)));
				}
				else {
					Exception.assert(typeof subRule === "object", "Rule for key '" + key + "' must be an object");
					Exception.assert(typeof object[key] === "object", () => ("Value for key '" + key + "' must be an object in "+ JSON.stringify(object)));
					for (const subKey in object[key])
					{
						validate(object[key], subKey, subRule);
					}
				}
			};

			// Must be a list of entries with relative timestamp
			for (const i in request.body.list) {

				const entry = request.body.list[i];
				const timestampDelta = entry.timestamp || 0;

				// Validate the type of entries
				let data = {};
				for (const key in entry) {
					validate(entry, key, validKeys);
					if (key == "timestamp") {
						continue;
					}
					data[key] = entry[key];
				}

				// Store the timestamp
				await this.options.timeseries.insert(timestamp - timestampDelta, {
					[NodeId]: data
				});
			}

			response.sendStatus(200);
		}, Web.JSON, {
			exceptionGuard: true
		});

		this.web.addRoute("get", "/api/v1/sample", async (request, response) => {

			// Set the options
			const options = Object.assign({
				from: undefined,
				until: undefined,
				max: 100
			}, request.query);

			// Return the last 24h (starting from the peviously registered point)
			const lastTimestamp = await this.options.timeseries.getTimestamp(-1);

			let dataList = [];
			await this.options.timeseries.forEach((timestamp, data) => {
				dataList.push([timestamp, data]);
			}, lastTimestamp - 24 * 60 * 60);

			response.json(dataList);

		}, Web.JSON, {
			exceptionGuard: true
		});
	}
};
