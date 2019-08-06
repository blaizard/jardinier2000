<template>
	<div>
		<button @click="addSample">Add Sample</button>
		<div v-for="data, index in dataPerType" :key="index">
			<CanvasJs :data="data"></CanvasJs>
		</div>
	</div>
</template>

<script>
	"use strict"

	import CanvasJs from "./canvasjs.vue"

	export default {
		components: {
			CanvasJs
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
			dataPerType() {
				let sensors = {};
				this.dataList.forEach((data) => {
					for (const sensorId in data[1]) {
						for (const type in data[1][sensorId]) {
							sensors[type] = sensors[type] || {};
							sensors[type][sensorId] = sensors[type][sensorId] || [];
							sensors[type][sensorId].push({
								x: new Date(data[0] * 1000),
								y: data[1][sensorId][type]
							});
							this.lastSample = data[1][sensorId][type];
						}
					}
				});
				return sensors;
			}
		},
		methods: {
			async fetchSensorData() {
				this.dataList = await this.fetch("/api/v1/sample", {
					method: "get",
					type: "json"
				});

				setTimeout(this.fetchSensorData, this.fetchPeriodMs);
			},
			async addSample() {

				this.lastSample += Math.random() * 2 - 1;

				await this.fetch("/api/v1/sample", {
					method: "post",
					query: {
						key: "SpMAtpJuNScdz77vh6D93V9Ays8KtMS0"
					},
					body: JSON.stringify({
						list: [{
							timestamp: 0,
							humidity: this.lastSample
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
</style>
