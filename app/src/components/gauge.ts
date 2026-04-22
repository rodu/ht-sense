import { LitElement, html, css } from 'lit';
import { customElement, property } from 'lit/decorators.js';
import * as echarts from 'echarts/core';
import { GaugeChart } from 'echarts/charts';
import { CanvasRenderer } from 'echarts/renderers';
import type { SensorReading } from '../types/sensor-reading';

echarts.use([GaugeChart, CanvasRenderer]);

@customElement('ht-gauge')
export class HtGauge extends LitElement {
  static styles = css`
    :host {
      display: block;
    }
    .chart-container {
      width: 100%;
      height: 450px;
    }
  `;

  @property({ type: Object }) reading: SensorReading | null = null;

  private chart: echarts.ECharts | null = null;

  protected firstUpdated(): void {
    const container =
      this.shadowRoot!.querySelector<HTMLElement>('.chart-container')!;
    this.chart = echarts.init(container);
    this.updateChart();
  }

  protected updated(): void {
    this.updateChart();
  }

  private updateChart(): void {
    if (!this.chart || !this.reading) return;

    this.chart.setOption({
      series: [
        {
          name: 'Temperature',
          type: 'gauge',
          min: -10,
          max: 50,
          splitNumber: 6,
          center: ['25%', '50%'],
          radius: '60%',
          progress: { show: true },
          detail: {
            valueAnimation: true,
            formatter: '{value} °C',
            offsetCenter: ['0%', '70%'],
          },
          title: { offsetCenter: ['0%', '40%'] },
          data: [
            { value: this.reading.temperatureCelsius, name: 'Temperature' },
          ],
        },
        {
          name: 'Humidity',
          type: 'gauge',
          min: 0,
          max: 100,
          center: ['75%', '50%'],
          radius: '60%',
          progress: { show: true },
          detail: {
            valueAnimation: true,
            formatter: '{value} %',
            offsetCenter: ['0%', '70%'],
          },
          title: { offsetCenter: ['0%', '40%'] },
          data: [{ value: this.reading.humidity, name: 'Humidity' }],
        },
      ],
    });
  }

  render() {
    return html`<div class="chart-container"></div>`;
  }
}

declare global {
  interface HTMLElementTagNameMap {
    'ht-gauge': HtGauge;
  }
}
