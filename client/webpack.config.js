const Path = require("path");
const Webpack = require("../lib/webpack.js");

let config = {
	entries: {
		index: Path.resolve(__dirname, "app.js")
	},
	output: Path.resolve(__dirname, "..", ".dist"),
	publicPath: "/",
	alias: {
		"[client]": Path.resolve(__dirname)
	},
	templates: {
		index: {
			entryId: "index",
			template: Path.resolve(__dirname, "template.index.html")
		}
	}
};

module.exports = Webpack.generate(config);
