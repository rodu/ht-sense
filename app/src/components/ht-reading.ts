import { LitElement, html } from 'lit';
import { customElement, state } from 'lit/decorators.js';
import { Subscription } from 'rxjs';
import { instance } from '../core/decorators';
import { ApiService } from '../services/api-service';
import type { SensorReading } from '../types/sensor-reading';
import './gauge';

@customElement('ht-reading')
export class HtReading extends LitElement {
  @instance({ type: ApiService })
  private apiService!: ApiService;

  @state() private reading: SensorReading | null = null;

  private sensorStreamSubscription: Subscription | null = null;

  connectedCallback(): void {
    super.connectedCallback();

    this.sensorStreamSubscription = this.apiService
      .streamSensorData()
      .subscribe((reading) => {
        this.reading = reading;
      });
  }

  disconnectedCallback(): void {
    this.sensorStreamSubscription?.unsubscribe();
    this.sensorStreamSubscription = null;

    super.disconnectedCallback();
  }

  render() {
    return html`<ht-gauge .reading=${this.reading}></ht-gauge>`;
  }
}

declare global {
  interface HTMLElementTagNameMap {
    'ht-reading': HtReading;
  }
}
