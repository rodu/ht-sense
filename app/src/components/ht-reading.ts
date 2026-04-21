import { LitElement, html } from 'lit';
import { customElement } from 'lit/decorators.js';
import { Subscription } from 'rxjs';
import { instance } from '../core/decorators';
import { ApiService } from '../services/api-service';

@customElement('ht-reading')
export class HtReading extends LitElement {
  @instance({ type: ApiService })
  private apiService!: ApiService;

  private sensorStreamSubscription: Subscription | null = null;

  connectedCallback(): void {
    super.connectedCallback();

    this.sensorStreamSubscription = this.apiService
      .streamSensorData()
      .subscribe((reading) => {
        console.log('Sensor reading:', reading);
      });
  }

  disconnectedCallback(): void {
    this.sensorStreamSubscription?.unsubscribe();
    this.sensorStreamSubscription = null;

    super.disconnectedCallback();
  }

  render() {
    return html`<div>Listening for sensor readings...</div>`;
  }
}

declare global {
  interface HTMLElementTagNameMap {
    'ht-reading': HtReading;
  }
}
