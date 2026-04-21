import { LitElement, html } from 'lit';
import { customElement } from 'lit/decorators.js';
import './components/ht-reading';

@customElement('my-element')
class MyElement extends LitElement {
  render() {
    return html`<ht-reading></ht-reading>`;
  }
}

declare global {
  interface HTMLElementTagNameMap {
    'my-element': MyElement;
  }
}
