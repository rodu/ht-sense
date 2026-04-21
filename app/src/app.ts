import { LitElement, css, html } from 'lit';
import { customElement, property, state } from 'lit/decorators.js';

import { ApiService, IUser } from './services/api-service';
import './components/user-detail';
import { EventAggregator, Subscription } from 'aurelia-event-aggregator';
import { instance } from './core/decorators';

@customElement('my-element')
class MyElement extends LitElement {
  @property({ type: String })
  classes = '';

  @instance({ type: ApiService })
  private apiService!: ApiService;
  @instance({ type: EventAggregator })
  private eventAggregator!: EventAggregator;

  @state()
  private users: IUser[] = [];
  @state()
  private selectedUser: IUser | null = null;

  private subscriptions: Subscription[] = [];

  async connectedCallback() {
    super.connectedCallback();

    this.subscriptions.push(
      this.eventAggregator.subscribe('USER_SELECTED', (user: IUser) => {
        this.selectedUser = user;
        this.requestUpdate();
      })
    );

    this.users = await this.apiService.loadUsers();
  }

  render() {
    console.log('Re-rendering element');

    return html` <div class="${this.classes}">
      ${renderSelectedUser(this.selectedUser)}
      ${this.users.map(renderUserDetail)}
    </div>`;
  }

  disconnectedCallback(): void {
    this.subscriptions.forEach((subscription) => subscription.dispose());
  }

  static styles = css`
    :host {
      max-width: 1280px;
      margin: 0 auto;
      padding: 2rem;
      text-align: center;
    }

    .selected {
      margin: 10px 0;
      padding: 10px;
      background-color: yellow;
      border: solid 1px #444;
    }

    .logo {
      height: 6em;
      padding: 1.5em;
      will-change: filter;
      transition: filter 300ms;
    }
    .logo:hover {
      filter: drop-shadow(0 0 2em #646cffaa);
    }
    .logo.lit:hover {
      filter: drop-shadow(0 0 2em #325cffaa);
    }

    .card {
      padding: 2em;
    }

    .read-the-docs {
      color: #888;
    }

    ::slotted(h1) {
      font-size: 3.2em;
      line-height: 1.1;
    }

    a {
      font-weight: 500;
      color: #646cff;
      text-decoration: inherit;
    }
    a:hover {
      color: #535bf2;
    }

    button {
      border-radius: 8px;
      border: 1px solid transparent;
      padding: 0.6em 1.2em;
      font-size: 1em;
      font-weight: 500;
      font-family: inherit;
      background-color: #1a1a1a;
      cursor: pointer;
      transition: border-color 0.25s;
    }
    button:hover {
      border-color: #646cff;
    }
    button:focus,
    button:focus-visible {
      outline: 4px auto -webkit-focus-ring-color;
    }

    @media (prefers-color-scheme: light) {
      a:hover {
        color: #747bff;
      }
      button {
        background-color: #f9f9f9;
      }
    }
  `;
}

declare global {
  interface HTMLElementTagNameMap {
    'my-element': MyElement;
  }
}

const renderUserDetail = (user: IUser) =>
  html`<user-detail .user=${user}></user-detail>`;

const renderSelectedUser = (user: IUser | null) =>
  user
    ? html`<div class="selected">
        Selected: ${user.firstName} ${user.lastName}
      </div>`
    : null;
