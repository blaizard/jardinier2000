<template>
	<div>
		<!--<Donut :value="donutValue" style="height: 370px; width: 100%;"></Donut>//-->
		Legend Right
		<Plot :value="plotValue" :config="{layout: 'legend-right'}" style="height: 300px; width: 100%;"></Plot>
		Legend Left
		<Plot :value="plotValue" :config="{layout: 'legend-left'}" style="height: 300px; width: 100%;"></Plot>
		Dates (format X)
		<Plot :value="plotValue" :config="{formatX: formatXToDate}" style="height: 300px; width: 100%;"></Plot>
		<button @click="addSample">Add Sample</button>
		<div v-for="data, index in dataPerType" :key="index">
			<CanvasJs :data="data" :title="index"></CanvasJs>
		</div>
	</div>
</template>

<script>
	"use strict"

	import CanvasJs from "./canvasjs.vue"
	import Donut from "[lib]/vue/components/graph/donut.vue"
	import Plot from "[lib]/vue/components/graph/plot.vue"

	export default {
		components: {
			CanvasJs, Donut, Plot
		},
		data: function() {
			return {
				dataList: [],
				fetchPeriodMs: 3000,
				lastSample: 20
			}
		},
		mounted() {
			this.fetchSensorData();
		},
		computed: {
			donutValue() {
				return [
					{caption: "Hello", value: 0.1, color: "red"},
					{caption: "Friend", value: 0.2, color: "blue"},
					{caption: "Me", value: 0.7, color: "green"}];
			},
			plotValue() {
				let series = [];
				for (const name in this.dataPerType["humidity"]) {
					series.push({
						caption: name,
						values: this.dataPerType["humidity"][name].map((value) => [value.x, value.y])
					});
				}
				return series;

				/*let series2 = [];
				for (const name in {"test-0": true, "test-1": true, "test-2": true, "test-3": true, "test-4": true, "test-5": true, "test-6": true, "test-7": true, "test-8": true, "test-9": true, "test-10": true, "test-11": true}) {
					let y = Math.random() * 50;
					let values = [];
					for (let x=0; x<10; ++x) {
						values.push([x + Math.random() - 0.5, y]);
						y += Math.random() * 2 - 1;
					}
					series2.push({
						caption: name,
						values: values
					});
				}

				return series2;*/
				/*return [
					{caption: "Hello", values: [[0, 0], [1, 5], [2, 31], [3, 45], [4, 1], [5, 56], [6, 23]], color: "red"}
				];*/
			},
			dataPerType() {
				let sensors = {};
				this.dataList.forEach((data) => {
					for (const sensorId in data[1]) {
						for (const type in data[1][sensorId]) {
							sensors[type] = sensors[type] || {};

							for (const sensorName in data[1][sensorId][type]) {
								const sensorVariantId = sensorId + " (" + sensorName + ")";
								sensors[type][sensorVariantId] = sensors[type][sensorVariantId] || [];
								sensors[type][sensorVariantId].push({
									x: new Date(data[0] * 1000),
									y: data[1][sensorId][type][sensorName]
								});
							}
						}
					}
				});
				return sensors;
			}
		},
		methods: {
			formatXToDate(x) {
				return (new Date(x)).toISOString().slice(-13, -5);
			},
			async fetchSensorData() {
				this.dataList = await this.fetch("/api/v1/sample", {
					method: "get",
					type: "json"
				});

				setTimeout(this.fetchSensorData, this.fetchPeriodMs);
			},
			async addSample() {
				await this.fetch("/api/v1/sample", {
					method: "post",
					query: {
						token: "SpMAtpJuNScdz77vh6D93V9Ays8KtMS0"
					},
					body: JSON.stringify({
						list: [{
							timestamp: 0,
							humidity: {test: (Math.random() * 255)}
						}]
					}),
					headers: {
						"Accept": "application/json",
						"Content-Type": "application/json"
					}
				});
			},
			errorHandler(e) {
				console.error(e);
			},
			async fetch(url, options) {
				options = Object.assign({
					query: null,
					method: "get",
					type: "text",
					body: undefined,
					headers: {}
				}, options);

				try {
					if (options.query) {
						const query = Object.keys(options.query).map((key) => key + "=" + encodeURIComponent(options.query[key])).join("&");
						url += ((query) ? ("?" + query) : "");
					}

					const response = await fetch(url, {
						method: options.method,
						body: options.body,
						headers: options.headers
					});

					if (!response.ok) {
						throw Error((await response.text()) || response.statusText);
					}
					let data = null;
					switch (options.type) {
					case "json":
						data = await response.json();
						break;
					default:
						data = await response.text();
					}
					return data;
				}
				catch (e) {
					this.errorHandler(e);
				}
			}
		}
	}
</script>

<style lang="scss">
	@import "~[lib]/vue/components/graph/style/index.scss";
</style>
