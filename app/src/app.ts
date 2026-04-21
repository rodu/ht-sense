import { LitElement, html } from 'lit';
import { customElement } from 'lit/decorators.js';
import './components/ht-reading';

@customElement('ht-sense')
class App extends LitElement {
  render() {
    return html`<ht-reading></ht-reading>`;
  }
}

declare global {
  interface HTMLElementTagNameMap {
    'ht-sense': App;
  }
}
