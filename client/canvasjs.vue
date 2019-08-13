<template>
	<div ref="chart" style="height: 370px; width: 100%;">
	</div>
</template>

<script>
	"use strict";

	export default {
		data: function() {
			return {
				chart: null,
				charConfig: {
					animationEnabled: true,
					theme: "light2",
					title:{
						text: this.title
					},
					axisX:{
						valueFormatString: "D MMM HH:mm",
						crosshair: {
							enabled: true,
							snapToDataPoint: true
						}
					},
					axisY:{
						includeZero: false
					},
					data: []
				}
			}
		},
		props: {
			data: {required: true},
			title: {required: true}
		},
		watch: {
			data() {
				let dataList = [];
				for (const name in this.data) {
					dataList.push({
						type: "spline",
						name: name,
						showInLegend: true,
						dataPoints: this.data[name]
					})
				}
				this.charConfig.data = dataList;
				this.chart.render();
			}
		},
		computed: {
			dataListChart() {
				let dataList = [];
				for (const name in this.data) {
					dataList.push({
						type: "line",
						name: name,
						showInLegend: true,
						dataPoints: this.data[name]
					})
				}
				return dataList;
			}
		},
		mounted() {
			this.chart = new CanvasJS.Chart(this.$refs.chart, this.charConfig);
		}
	}
</script>